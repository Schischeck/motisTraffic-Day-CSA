#include "motis/loader/graph_builder.h"

#include <cassert>
#include <algorithm>
#include <functional>

#include "parser/cstr.h"

#include "motis/core/common/constants.h"
#include "motis/core/common/get_or_create.h"
#include "motis/core/common/logging.h"
#include "motis/core/schedule/category.h"
#include "motis/core/schedule/price.h"
#include "motis/loader/classes.h"
#include "motis/loader/duplicate_checker.h"
#include "motis/loader/rule_service_graph_builder.h"
#include "motis/loader/timezone_util.h"
#include "motis/loader/util.h"
#include "motis/loader/wzr_loader.h"

using namespace motis::logging;
using namespace flatbuffers;

namespace motis {
namespace loader {

graph_builder::graph_builder(schedule& sched, Interval const* schedule_interval,
                             time_t from, time_t to, bool apply_rules,
                             bool adjust_footpaths)
    : next_route_index_(-1),
      next_node_id_(-1),
      sched_(sched),
      first_day_((from - schedule_interval->from()) / (MINUTES_A_DAY * 60)),
      last_day_((to - schedule_interval->from()) / (MINUTES_A_DAY * 60) - 1),
      apply_rules_(apply_rules),
      adjust_footpaths_(adjust_footpaths) {
  if (to <= from || schedule_interval->from() >= static_cast<uint64_t>(to) ||
      schedule_interval->to() <= static_cast<uint64_t>(from)) {
    throw std::runtime_error("schedule out of bounds");
  }

  connections_.set_empty_key(nullptr);
  con_infos_.set_empty_key(nullptr);
  bitfields_.set_empty_key(nullptr);
  stations_.set_empty_key(nullptr);
  duplicate_count_ = 0;
}

void graph_builder::add_stations(Vector<Offset<Station>> const* stations) {
  // Add dummy source station.
  auto dummy_source =
      make_unique<station>(0, 0.0, 0.0, 0, "-1", "DUMMY", nullptr);
  sched_.eva_to_station_.insert(
      std::make_pair(dummy_source->eva_nr_, dummy_source.get()));
  sched_.stations_.emplace_back(std::move(dummy_source));
  sched_.station_nodes_.emplace_back(make_unique<station_node>(0));

  // Add dummy target stations.
  auto dummy_target =
      make_unique<station>(1, 0.0, 0.0, 0, "-2", "DUMMY", nullptr);
  sched_.eva_to_station_.insert(
      std::make_pair(dummy_target->eva_nr_, dummy_target.get()));
  sched_.stations_.emplace_back(std::move(dummy_target));
  sched_.station_nodes_.emplace_back(make_unique<station_node>(1));

  // Add schedule stations.
  for (unsigned i = 0; i < stations->size(); ++i) {
    auto const& input_station = stations->Get(i);
    auto const station_index = i + 2;

    // Create station node.
    auto node_ptr = make_unique<station_node>(station_index);
    stations_[input_station] = node_ptr.get();
    sched_.station_nodes_.emplace_back(std::move(node_ptr));

    // Create station object.
    auto s = make_unique<station>();
    s->index_ = station_index;
    s->name_ = input_station->name()->str();
    s->width_ = input_station->lat();
    s->length_ = input_station->lng();
    s->eva_nr_ = input_station->id()->str();
    s->transfer_time_ = input_station->interchange_time();
    s->timez_ = input_station->timezone()
                    ? get_or_create_timezone(input_station->timezone())
                    : nullptr;
    sched_.eva_to_station_.insert(
        std::make_pair(input_station->id()->str(), s.get()));
    sched_.stations_.emplace_back(std::move(s));

    // Store DS100.
    if (input_station->external_ids()) {
      for (auto const& ds100 : *input_station->external_ids()) {
        sched_.ds100_to_station_.insert(std::make_pair(ds100->str(), s.get()));
      }
    }
  }

  // First regular node id:
  // first id after station node ids
  next_node_id_ = sched_.stations_.size();
}

void graph_builder::link_meta_stations(
    Vector<Offset<MetaStation>> const* meta_stations) {
  for (auto const& meta : *meta_stations) {
    auto station = sched_.eva_to_station_.find(meta->station()->id()->str());
    for (auto const& fbs_equivalent : *meta->equivalent()) {
      auto equivalent =
          sched_.eva_to_station_.find(fbs_equivalent->id()->str());
      station->second->equivalent_.push_back(equivalent->second);
    }
  }
}

timezone const* graph_builder::get_or_create_timezone(
    Timezone const* input_timez) {
  return get_or_create(timezones_, input_timez, [&]() {
    auto const tz =
        input_timez->season()
            ? create_timezone(
                  input_timez->general_offset(),
                  input_timez->season()->offset(),  //
                  first_day_, last_day_,
                  input_timez->season()->day_idx_first_day(),
                  input_timez->season()->day_idx_last_day(),
                  input_timez->season()->minutes_after_midnight_first_day(),
                  input_timez->season()->minutes_after_midnight_last_day())
            : timezone(input_timez->general_offset());
    sched_.timezones_.emplace_back(new timezone(tz));
    return sched_.timezones_.back().get();
  });
}

station_node* graph_builder::get_station_node(Station const* station) const {
  auto it = stations_.find(station);
  verify(it != end(stations_), "station not found");
  return it->second;
}

full_trip_id graph_builder::get_full_trip_id(Service const* s, int day,
                                             int section_idx) const {
  auto const& stops = s->route()->stations();
  auto const dep_station_idx = get_station_node(stops->Get(section_idx))->id_;
  auto const arr_station_idx =
      get_station_node(stops->Get(stops->size() - 1))->id_;

  auto const dep_tz = sched_.stations_[dep_station_idx]->timez_;
  auto const dep_time = get_adjusted_event_time(
      day - first_day_, s->times()->Get(section_idx * 2 + 1), dep_tz);

  auto const arr_tz = sched_.stations_[arr_station_idx]->timez_;
  auto const arr_time = get_adjusted_event_time(
      day - first_day_, s->times()->Get(s->times()->size() - 2), arr_tz);

  auto const train_nr = s->sections()->Get(section_idx)->train_nr();
  auto const line_id_ptr = s->sections()->Get(0)->line_id();
  auto const line_id = line_id_ptr ? line_id_ptr->str() : "";

  full_trip_id id;
  id.primary_ = primary_trip_id(dep_station_idx, train_nr, dep_time);
  id.secondary_ = secondary_trip_id(arr_station_idx, arr_time, false, line_id);
  return id;
}

merged_trips_idx graph_builder::create_merged_trips(Service const* s,
                                                    int day_idx) {
  return push_mem(sched_.merged_trips_,
                  std::vector<trip*>({register_service(s, day_idx)}));
}

trip* graph_builder::register_service(Service const* s, int day_idx) {
  sched_.trip_mem_.emplace_back(new trip(get_full_trip_id(s, day_idx)));
  auto stored = sched_.trip_mem_.back().get();
  sched_.trips_[stored->id_.primary_].push_back(stored);

  for (unsigned i = 1; i < s->sections()->size(); ++i) {
    auto curr_section = s->sections()->Get(i);
    auto prev_section = s->sections()->Get(i - 1);

    if (curr_section->train_nr() != prev_section->train_nr()) {
      sched_.trips_[get_full_trip_id(s, day_idx, i).primary_].push_back(stored);
    }
  }

  if (s->initial_train_nr() != stored->id_.primary_.train_nr_) {
    auto primary = stored->id_.primary_;
    primary.train_nr_ = s->initial_train_nr();
    sched_.trips_[primary].push_back(stored);
  }

  return stored;
}

void graph_builder::add_services(Vector<Offset<Service>> const* services) {
  std::vector<Service const*> sorted(services->size());
  std::copy(std::begin(*services), std::end(*services), begin(sorted));
  std::sort(begin(sorted), end(sorted),
            [](Service const* lhs, Service const* rhs) {
              return lhs->route() < rhs->route();
            });

  auto it = begin(sorted);
  std::vector<Service const*> route_services;
  while (it != end(sorted)) {
    auto route = (*it)->route();
    do {
      if (!apply_rules_ || !(*it)->rule_participant()) {
        route_services.push_back(*it);
      }
      ++it;
    } while (it != end(sorted) && route == (*it)->route());

    if (!route_services.empty()) {
      add_route_services(route_services);
    }

    route_services.clear();
  }
}

void graph_builder::index_first_route_node(route const& r) {
  assert(!r.empty());
  auto route_index = r[0].from_route_node_->route_;
  if (static_cast<int>(sched_.route_index_to_first_route_node_.size()) <=
      route_index) {
    sched_.route_index_to_first_route_node_.resize(route_index + 1);
  }
  sched_.route_index_to_first_route_node_[route_index] = r[0].from_route_node_;
}

void graph_builder::add_route_services(
    std::vector<Service const*> const& services) {
  if (services.empty()) {
    return;
  }

  std::vector<route_lcs> alt_routes;
  for (auto const& s : services) {
    auto const traffic_days = get_or_create_bitfield(s->traffic_days());
    auto const first_day_offset =
        s->times()->Get(s->times()->size() - 2) / 1440;
    auto const first_day = std::max(0, first_day_ - first_day_offset);

    for (int day = first_day; day <= last_day_; ++day) {
      if (!traffic_days.test(day)) {
        continue;
      }

      time prev_arr = 0;
      bool adjusted = false;
      std::vector<light_connection> lcons;
      auto t = create_merged_trips(s, day);
      for (int section_idx = 0;
           section_idx < static_cast<int>(s->sections()->size());
           ++section_idx) {
        lcons.push_back(section_to_connection(
            t, {{participant{s, section_idx}}}, day, prev_arr, adjusted));
        prev_arr = lcons.back().a_time_;
      }

      add_to_routes(alt_routes, lcons);
    }
  }

  for (auto const& route : alt_routes) {
    if (route.empty() || route[0].empty()) {
      continue;
    }

    auto r = create_route(services[0]->route(), route, ++next_route_index_);
    index_first_route_node(*r);
    write_trip_info(*r);
  }
}

int graph_builder::get_index(
    std::vector<std::vector<light_connection>> const& alt_route,
    std::vector<light_connection> const& sections) {
  assert(!sections.empty());
  assert(!alt_route.empty());

  if (alt_route[0].empty()) {
    return 0;
  }

  int index = -1;
  for (unsigned section_idx = 0; section_idx < sections.size(); ++section_idx) {
    auto const& route_section = alt_route[section_idx];
    auto const& lc = sections[section_idx];
    if (index == -1) {
      index = std::distance(
          begin(route_section),
          std::lower_bound(begin(route_section), end(route_section),
                           sections[section_idx]));
    } else {
      // Check if departures stay sorted.
      bool earlier_eq_dep =
          index > 0 && lc.d_time_ <= route_section[index - 1].d_time_;
      bool later_eq_dep = static_cast<unsigned>(index) < route_section.size() &&
                          lc.d_time_ >= route_section[index].d_time_;

      // Check if arrivals stay sorted.
      bool earlier_eq_arr =
          index > 0 && lc.a_time_ <= route_section[index - 1].a_time_;
      bool later_eq_arr = static_cast<unsigned>(index) < route_section.size() &&
                          lc.a_time_ >= route_section[index].a_time_;

      if (earlier_eq_dep || later_eq_dep || earlier_eq_arr || later_eq_arr) {
        return -1;
      }
    }
  }
  return index;
}

void graph_builder::add_to_route(
    std::vector<std::vector<light_connection>>& route,
    std::vector<light_connection> const& sections, int index) {
  for (unsigned section_idx = 0; section_idx < sections.size(); ++section_idx) {
    route[section_idx].insert(std::next(begin(route[section_idx]), index),
                              sections[section_idx]);
  }
}

void graph_builder::add_to_routes(
    std::vector<std::vector<std::vector<light_connection>>>& alt_routes,
    std::vector<light_connection> const& sections) {
  for (auto& alt_route : alt_routes) {
    int index = get_index(alt_route, sections);
    if (index == -1) {
      continue;
    }

    add_to_route(alt_route, sections, index);
    return;
  }

  alt_routes.push_back(
      std::vector<std::vector<light_connection>>(sections.size()));
  add_to_route(alt_routes.back(), sections, 0);
}

connection_info* graph_builder::get_or_create_connection_info(
    Section const* section, int dep_day_index, connection_info* merged_with) {
  con_info_.line_identifier_ =
      section->line_id() ? section->line_id()->str() : "";
  con_info_.train_nr_ = section->train_nr();
  con_info_.family_ = get_or_create_category_index(section->category());
  con_info_.dir_ = get_or_create_direction(section->direction());
  con_info_.provider_ = get_or_create_provider(section->provider());
  con_info_.merged_with_ = merged_with;
  read_attributes(dep_day_index, section->attributes(), con_info_.attributes_);

  return set_get_or_create(con_infos_, &con_info_, [&]() {
    sched_.connection_infos_.emplace_back(
        make_unique<connection_info>(con_info_));
    return sched_.connection_infos_.back().get();
  });
}

connection_info* graph_builder::get_or_create_connection_info(
    std::array<participant, 16> const& services, int dep_day_index) {
  connection_info* prev_con_info = nullptr;

  for (auto service : services) {
    if (service.service_ == nullptr) {
      return prev_con_info;
    }

    auto const& s = service;
    prev_con_info = get_or_create_connection_info(
        s.service_->sections()->Get(s.section_idx_), dep_day_index,
        prev_con_info);
  }

  return prev_con_info;
}

light_connection graph_builder::section_to_connection(
    merged_trips_idx trips, std::array<participant, 16> const& services,
    int day, time prev_arr, bool& adjusted) {
  auto const& ref = services[0].service_;
  auto const& section_idx = services[0].section_idx_;

  assert(ref != nullptr);
  assert(std::all_of(begin(services), end(services), [&](participant const& s) {
    if (s.service_ == nullptr) {
      return true;
    }

    auto const& ref_stops = ref->route()->stations();
    auto const& s_stops = s.service_->route()->stations();
    auto stations_match =
        s_stops->Get(s.section_idx_) == ref_stops->Get(section_idx) &&
        s_stops->Get(s.section_idx_ + 1) == ref_stops->Get(section_idx + 1);

    auto times_match = s.service_->times()->Get(s.section_idx_ * 2 + 1) ==
                           ref->times()->Get(section_idx * 2 + 1) &&
                       s.service_->times()->Get(s.section_idx_ * 2 + 2) ==
                           ref->times()->Get(section_idx * 2 + 2);

    return stations_match && times_match;
  }));
  assert(std::is_sorted(begin(services), end(services)));

  auto& from = *sched_.stations_.at(
      stations_[ref->route()->stations()->Get(section_idx)]->id_);
  auto& to = *sched_.stations_.at(
      stations_[ref->route()->stations()->Get(section_idx + 1)]->id_);

  auto plfs = ref->platforms();
  auto dep_platf = plfs ? plfs->Get(section_idx)->dep_platforms() : nullptr;
  auto arr_platf = plfs ? plfs->Get(section_idx + 1)->arr_platforms() : nullptr;

  auto section = ref->sections()->Get(section_idx);
  auto dep_time = ref->times()->Get(section_idx * 2 + 1);
  auto arr_time = ref->times()->Get(section_idx * 2 + 2);

  // Day indices for shifted bitfields (platforms, attributes)
  int dep_day_index = day + (dep_time / MINUTES_A_DAY);
  int arr_day_index = day + (arr_time / MINUTES_A_DAY);

  // Build full connection.
  auto clasz_it = sched_.classes_.find(section->category()->name()->str());
  con_.clasz_ = (clasz_it == end(sched_.classes_)) ? 9 : clasz_it->second;
  con_.price_ = get_distance(from, to) * get_price_per_km(con_.clasz_);
  con_.d_platform_ = get_or_create_platform(dep_day_index, dep_platf);
  con_.a_platform_ = get_or_create_platform(arr_day_index, arr_platf);
  con_.con_info_ = get_or_create_connection_info(services, dep_day_index);

  // Build light connection.
  time dep_motis_time, arr_motis_time;
  std::tie(dep_motis_time, arr_motis_time) =
      get_event_times(day - first_day_, prev_arr, dep_time, arr_time,
                      from.timez_, to.timez_, adjusted);

  // Count events.
  ++from.dep_class_events_.at(con_.clasz_);
  ++to.arr_class_events_.at(con_.clasz_);

  return light_connection(
      dep_motis_time, arr_motis_time,
      set_get_or_create(connections_, &con_,
                        [&]() {
                          sched_.full_connections_.emplace_back(
                              make_unique<connection>(con_));
                          return sched_.full_connections_.back().get();
                        }),
      trips);
}

void graph_builder::add_footpaths(Vector<Offset<Footpath>> const* footpaths) {
  for (auto const& footpath : *footpaths) {
    auto duration = footpath->duration();
    auto from_node = stations_[footpath->from()];
    auto to_node = stations_[footpath->to()];
    auto const& from_station = *sched_.stations_.at(from_node->id_);
    auto const& to_station = *sched_.stations_.at(to_node->id_);

    if (adjust_footpaths_) {
      uint32_t max_transfer_time =
          std::max(from_station.transfer_time_, to_station.transfer_time_);

      duration = std::max(max_transfer_time, footpath->duration());

      auto const distance = get_distance(from_station, to_station) * 1000;
      auto const max_distance_adjust = duration * 60 * WALK_SPEED;
      auto const max_distance = 2 * duration * 60 * WALK_SPEED;

      if (distance > max_distance) {
        continue;
      } else if (distance > max_distance_adjust) {
        duration = std::round(distance / (60 * WALK_SPEED));
      }

      if (duration > 15) {
        continue;
      }
    }

    next_node_id_ = from_node->add_foot_edge(
        next_node_id_, make_foot_edge(from_node, to_node, duration));
  }
}

void graph_builder::connect_reverse() {
  for (auto& station_node : sched_.station_nodes_) {
    for (auto& station_edge : station_node->edges_) {
      station_edge.to_->incoming_edges_.push_back(&station_edge);
      for (auto& edge : station_edge.to_->edges_) {
        edge.to_->incoming_edges_.push_back(&edge);
      }
    }
  }
}

void graph_builder::sort_connections() {
  for (auto& station_node : sched_.station_nodes_) {
    for (auto& station_edge : station_node->edges_) {
      for (auto& edge : station_edge.to_->edges_) {
        if (edge.empty()) {
          continue;
        }
        std::sort(begin(edge.m_.route_edge_.conns_),
                  end(edge.m_.route_edge_.conns_));
      }
    }
  }
}

int graph_builder::node_count() const { return next_node_id_; }

bitfield const& graph_builder::get_or_create_bitfield(
    String const* serialized_bitfield) {
  return map_get_or_create(bitfields_, serialized_bitfield, [&]() {
    return deserialize_bitset<BIT_COUNT>(
        {serialized_bitfield->c_str(), serialized_bitfield->Length()});
  });
}

void graph_builder::read_attributes(
    int day, Vector<Offset<Attribute>> const* attributes,
    std::vector<attribute const*>& active_attributes) {
  active_attributes.clear();
  active_attributes.reserve(attributes->size());
  for (auto const& attr : *attributes) {
    if (!get_or_create_bitfield(attr->traffic_days()).test(day)) {
      continue;
    }
    auto const attribute_info = attr->info();
    active_attributes.push_back(
        get_or_create(attributes_, attribute_info, [&]() {
          auto new_attr = make_unique<attribute>();
          new_attr->code_ = attribute_info->code()->str();
          new_attr->str_ = attribute_info->text()->str();
          sched_.attributes_.emplace_back(std::move(new_attr));
          return sched_.attributes_.back().get();
        }));
  }
}

std::string const* graph_builder::get_or_create_direction(
    Direction const* dir) {
  if (dir == nullptr) {
    return nullptr;
  } else if (dir->station()) {
    return &sched_.stations_[stations_[dir->station()]->id_]->name_;
  } else /* direction text */ {
    return get_or_create(directions_, dir->text(), [&]() {
      sched_.directions_.emplace_back(
          make_unique<std::string>(dir->text()->str()));
      return sched_.directions_.back().get();
    });
  }
}

provider const* graph_builder::get_or_create_provider(Provider const* p) {
  if (p == nullptr) {
    return nullptr;
  } else {
    return get_or_create(providers_, p, [&]() {
      sched_.providers_.emplace_back(make_unique<provider>(
          provider(p->short_name()->str(), p->long_name()->str(),
                   p->full_name()->str())));
      return sched_.providers_.back().get();
    });
  }
}

int graph_builder::get_or_create_category_index(Category const* c) {
  return get_or_create(categories_, c, [&]() {
    int index = sched_.categories_.size();
    sched_.categories_.push_back(make_unique<category>(
        category(c->name()->str(), static_cast<uint8_t>(c->output_rule()))));
    return index;
  });
}

int graph_builder::get_or_create_platform(
    int day, Vector<Offset<Platform>> const* platforms) {
  static constexpr int no_track = 0;
  if (sched_.tracks_.empty()) {
    sched_.tracks_.push_back("");
  }

  if (platforms == nullptr) {
    return no_track;
  }

  auto track_it = std::find_if(
      std::begin(*platforms), std::end(*platforms), [&](Platform const* track) {
        return get_or_create_bitfield(track->bitfield()).test(day);
      });
  if (track_it == std::end(*platforms)) {
    return no_track;
  } else {
    auto name = track_it->name()->str();
    return get_or_create(tracks_, name, [&]() {
      int index = sched_.tracks_.size();
      sched_.tracks_.push_back(name);
      return index;
    });
  }
}

void graph_builder::write_trip_info(route& r) {
  auto const edges = transform_to_vec(
      begin(r), end(r), [](route_section& s) { return s.get_route_edge(); });

  sched_.trip_edges_.emplace_back(new std::vector<edge*>(edges));
  auto edges_ptr = sched_.trip_edges_.back().get();

  auto& lcons = edges_ptr->front()->m_.route_edge_.conns_;
  for (unsigned lcon_idx = 0; lcon_idx < lcons.size(); ++lcon_idx) {
    auto trp = sched_.merged_trips_[lcons[lcon_idx].trips_]->front();
    trp->edges_ = edges_ptr;
    trp->lcon_idx_ = lcon_idx;
  }
}

std::unique_ptr<route> graph_builder::create_route(Route const* r,
                                                   route_lcs const& lcons,
                                                   unsigned route_index) {
  auto const& stops = r->stations();
  auto const& in_allowed = r->in_allowed();
  auto const& out_allowed = r->out_allowed();
  auto route_sections = make_unique<route>();

  route_section last_route_section;
  for (unsigned i = 0; i < r->stations()->size() - 1; ++i) {
    auto from = i;
    auto to = i + 1;
    route_sections->push_back(add_route_section(
        route_index, lcons[i],  //
        stops->Get(from), in_allowed->Get(from), out_allowed->Get(from),
        stops->Get(to), in_allowed->Get(to), out_allowed->Get(to),
        last_route_section, route_section()));
    last_route_section = route_sections->back();
  }

  return route_sections;
}

node* graph_builder::build_route_node(int route_index, Station const* station,
                                      bool in_allowed, bool out_allowed) {
  auto station_node = stations_[station];

  auto route_node = new node(station_node, next_node_id_++);
  route_node->route_ = route_index;

  if (!in_allowed) {
    station_node->edges_.push_back(make_invalid_edge(station_node, route_node));
  } else {
    station_node->edges_.push_back(make_foot_edge(station_node, route_node));
  }

  if (!out_allowed) {
    route_node->edges_.push_back(make_invalid_edge(route_node, station_node));
  } else {
    route_node->edges_.push_back(make_foot_edge(
        route_node, station_node,
        sched_.stations_[station_node->id_]->transfer_time_, true));
  }

  return route_node;
}

route_section graph_builder::add_route_section(
    int route_index, std::vector<light_connection> const& connections,
    Station const* from_stop, bool from_in_allowed, bool from_out_allowed,
    Station const* to_stop, bool to_in_allowed, bool to_out_allowed,
    route_section prev_section, route_section next_section) {
  route_section section;

  if (prev_section.is_valid()) {
    section.from_route_node_ = prev_section.to_route_node_;
  } else {
    section.from_route_node_ = build_route_node(
        route_index, from_stop, from_in_allowed, from_out_allowed);
  }

  if (next_section.is_valid()) {
    section.to_route_node_ = next_section.from_route_node_;
  } else {
    section.to_route_node_ =
        build_route_node(route_index, to_stop, to_in_allowed, to_out_allowed);
  }

  section.outgoing_route_edge_index_ = section.from_route_node_->edges_.size();
  section.from_route_node_->edges_.push_back(make_route_edge(
      section.from_route_node_, section.to_route_node_, connections));

  return section;
}

schedule_ptr build_graph(Schedule const* serialized, time_t from, time_t to,
                         bool unique_check, bool apply_rules,
                         bool adjust_footpaths) {
  scoped_timer timer("building graph");

  schedule_ptr sched(new schedule());
  sched->classes_ = class_mapping();
  sched->schedule_begin_ = from;
  sched->schedule_end_ = to;

  graph_builder builder(*sched, serialized->interval(), from, to, apply_rules,
                        adjust_footpaths);
  builder.add_stations(serialized->stations());
  if (serialized->meta_stations() != nullptr) {
    builder.link_meta_stations(serialized->meta_stations());
  }
  builder.add_footpaths(serialized->footpaths());
  builder.add_services(serialized->services());

  if (apply_rules) {
    scoped_timer timer("rule services");
    rule_service_graph_builder rsgb(builder);
    rsgb.add_rule_services(serialized->rule_services());
  }

  builder.connect_reverse();
  builder.sort_connections();

  sched->node_count_ = builder.node_count();
  sched->lower_bounds_ = constant_graph(sched->station_nodes_);
  sched->waiting_time_rules_ = load_waiting_time_rules(sched->categories_);
  sched->schedule_begin_ -= SCHEDULE_OFFSET_MINUTES * 60;

  if (unique_check) {
    scoped_timer timer("unique check");
    duplicate_checker dup_checker(*sched);
    dup_checker.remove_duplicates();
    LOG(info) << "removed " << dup_checker.get_duplicate_count()
              << " duplicate events";
  }

  LOG(info) << sched->connection_infos_.size() << " connection infos";

  return sched;
}

}  // namespace loader
}  // namespace motis
