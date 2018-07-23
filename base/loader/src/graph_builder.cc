#include "motis/loader/graph_builder.h"

#include <motis/core/schedule/edges.h>
#include <numeric>

#include "utl/get_or_create.h"
#include "utl/to_vec.h"

#include "motis/core/common/logging.h"
#include "motis/core/schedule/constant_graph.h"
#include "motis/core/schedule/graph_build_utils.h"
#include "motis/core/schedule/price.h"
#include "motis/loader/classes.h"
#include "motis/loader/timezone_util.h"
#include "motis/loader/tracking_dedup.h"
#include "motis/loader/wzr_loader.h"

using namespace motis::logging;
using namespace flatbuffers64;

namespace motis {
namespace loader {

graph_builder::graph_builder(schedule& sched, Interval const* schedule_interval,
                             time_t from, time_t to, bool apply_rules,
                             bool adjust_footpaths)
    : sched_(sched),
      first_day_(0),
      last_day_(((schedule_interval->to() - schedule_interval->from()) /
                 (MINUTES_A_DAY * 60)) -
                1),
      from_day_((from - schedule_interval->from()) / (3600 * 24)),
      to_day_(from_day_ + ((to - from) / (3600 * 24))),
      apply_rules_(apply_rules),
      adjust_footpaths_(adjust_footpaths),
      next_node_id_(0),
      next_route_index_(0),
      lcon_count_(0) {
  connections_.set_empty_key(nullptr);
  con_infos_.set_empty_key(nullptr);
  stations_.set_empty_key(nullptr);
}

void graph_builder::add_dummy_node(std::string const& name) {
  auto const station_idx = sched_.station_nodes_.size();
  auto s =
      std::make_unique<station>(station_idx, 0.0, 0.0, 0, name, name, nullptr);
  sched_.eva_to_station_.insert(std::make_pair(name, s.get()));
  sched_.stations_.emplace_back(std::move(s));
  sched_.station_nodes_.emplace_back(
      std::make_unique<station_node>(station_idx));
}

void graph_builder::add_stations(Vector<Offset<Station>> const* stations) {
  // Add dummy stations.
  add_dummy_node(STATION_START);
  add_dummy_node(STATION_END);
  add_dummy_node(STATION_VIA0);
  add_dummy_node(STATION_VIA1);
  add_dummy_node(STATION_VIA2);
  add_dummy_node(STATION_VIA3);
  add_dummy_node(STATION_VIA4);
  add_dummy_node(STATION_VIA5);
  add_dummy_node(STATION_VIA6);
  add_dummy_node(STATION_VIA7);
  add_dummy_node(STATION_VIA8);
  add_dummy_node(STATION_VIA9);

  // Add schedule stations.
  auto const dummy_station_count = sched_.station_nodes_.size();
  for (unsigned i = 0; i < stations->size(); ++i) {
    auto const& input_station = stations->Get(i);
    auto const station_index = i + dummy_station_count;

    // Create station node.
    auto node_ptr = std::make_unique<station_node>(station_index);
    stations_[input_station] = node_ptr.get();
    sched_.station_nodes_.emplace_back(std::move(node_ptr));

    // Create station object.
    auto s = std::make_unique<station>();
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

    // Store DS100.
    if (input_station->external_ids()) {
      for (auto const& ds100 : *input_station->external_ids()) {
        sched_.ds100_to_station_.insert(std::make_pair(ds100->str(), s.get()));
      }
    }

    sched_.stations_.emplace_back(std::move(s));
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
  return utl::get_or_create(timezones_, input_timez, [&]() {
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

void graph_builder::add_services(
    flatbuffers64::Vector<flatbuffers64::Offset<Service>> const* services) {
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

void graph_builder::add_route_services(
    std::vector<Service const*> const& services) {
  if (services.empty()) {
    return;
  }

  std::vector<route_t> alt_routes;
  for (auto const& service : services) {
    auto bf = service->traffic_days();
    bitfield const traffic_days =
        deserialize_bitset<BIT_COUNT>({bf->c_str(), bf->size()});

    // skip services with no traffic within the time span
    bool traffic = false;
    int const day_offset =
        service->times()->Get(service->times()->size() - 2) / MINUTES_A_DAY;
    auto start_idx = std::max(0, static_cast<int>(from_day_) - day_offset);
    auto end_idx = std::min(BIT_COUNT, static_cast<unsigned>(to_day_));
    for (unsigned day_idx = start_idx; day_idx < end_idx; ++day_idx) {
      if (traffic_days.test(day_idx)) {
        traffic = true;
        break;
      }
    }
    if (!traffic) {
      continue;
    }

    // service times converted to utc relative to begin of day of first event
    std::vector<std::pair<std::vector<time>, std::unordered_set<unsigned>>>
        utc_times;

    try {
      utc_times =
          service_times_to_utc(traffic_days, start_idx, end_idx, service);
    } catch (...) {
      std::cerr << "\nBAD SERVICE\n";
      /*
            for (int i = 0; i < service->times()->size(); ++i) {
              std::cerr << service->times()->Get(i) << " ";
            }
            std::cerr << "\n";

            for (int i = 0; i < service->route()->stations()->size(); ++i) {
              auto const& station = service->route()->stations()->Get(i);

              auto const& arr = service->times()->Get(i * 2);
              auto const& dep = service->times()->Get(i * 2 + 1);

              std::cerr << station->id()->str() << " " <<
         station->name()->str(); if (i != 0) { std::cerr << " 0" <<
         std::setfill('0') << std::setw(2) << arr / 60
                          << std::setfill('0') << std::setw(2) << arr % 60;
              }
              if (i != service->route()->stations()->size() - 1) {
                std::cerr << " 0" << std::setfill('0') << std::setw(2) << dep /
         60
                          << std::setfill('0') << std::setw(2) << dep % 60;
              }
              std::cerr << "\n";
            }
            // throw;*/
      continue;  // skip broken service
    }

    // make lcon for each section and time string
    std::vector<std::vector<light_connection>> lcon_strings(utc_times.size());
    auto trip_idx = create_merged_trips(service, 0);
    for (unsigned time_idx = 0; time_idx < utc_times.size(); ++time_idx) {
      auto const section_length = static_cast<int>(service->sections()->size());
      lcon_strings[time_idx].resize(section_length);
      for (int section_idx = 0; section_idx < section_length; ++section_idx) {
        lcon_strings[time_idx][section_idx] = section_to_connection(
            section_idx, service, utc_times[time_idx].first,
            utc_times[time_idx].second, trip_idx);
        count_events(service, section_idx);
      }
    }

    for (unsigned i = 0; i < utc_times.size(); ++i) {
      auto const& times = utc_times[i].first;
      auto const& lcons = lcon_strings[i];
      add_to_routes(alt_routes, times, lcons);
    }

    //    for (auto const& lcon_string : lcon_strings) {
    //      std::cout << "\n";
    //      for (auto const& lcon : lcon_string) {
    //        std::cout << std::setfill(' ') << std::setw(4) << lcon.d_time_ <<
    //        " ";
    //        std::cout << std::setfill(' ') << std::setw(4) << lcon.a_time_ <<
    //        " ";
    //      }
    //    }
  }

  for (auto const& route : alt_routes) {
    if (route.empty()) {
      continue;
    }

    auto r = create_route(services[0]->route(), route, next_route_index_++);
    index_first_route_node(*r);
    write_trip_info(*r);
  }
}

void graph_builder::count_events(const Service* const& service,
                                 int section_idx) {
  auto& from = *sched_.stations_.at(
      stations_[service->route()->stations()->Get(section_idx)]->id_);
  auto& to = *sched_.stations_.at(
      stations_[service->route()->stations()->Get(section_idx + 1)]->id_);
  ++from.dep_class_events_.at(con_.clasz_);
  ++to.arr_class_events_.at(con_.clasz_);
}

merged_trips_idx graph_builder::create_merged_trips(Service const* s,
                                                    int day_idx) {
  return push_mem(sched_.merged_trips_,
                  std::vector<trip*>({register_service(s, day_idx)}));
}

trip* graph_builder::register_service(Service const* s, int day_idx) {
  sched_.trip_mem_.emplace_back(new trip(get_full_trip_id(s, day_idx)));
  auto stored = sched_.trip_mem_.back().get();
  sched_.trips_.emplace_back(stored->id_.primary_, stored);

  for (unsigned i = 1; i < s->sections()->size(); ++i) {
    auto curr_section = s->sections()->Get(i);
    auto prev_section = s->sections()->Get(i - 1);

    if (curr_section->train_nr() != prev_section->train_nr()) {
      sched_.trips_.emplace_back(get_full_trip_id(s, day_idx, i).primary_,
                                 stored);
    }
  }

  if (s->initial_train_nr() != stored->id_.primary_.train_nr_) {
    auto primary = stored->id_.primary_;
    primary.train_nr_ = s->initial_train_nr();
    sched_.trips_.emplace_back(primary, stored);
  }

  return stored;
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
  id.secondary_ = secondary_trip_id(arr_station_idx, arr_time.ts(), line_id);
  return id;
}

station_node* graph_builder::get_station_node(Station const* station) const {
  auto it = stations_.find(station);
  verify(it != end(stations_), "station not found");
  return it->second;
}

light_connection graph_builder::section_to_connection(
    unsigned section_idx, Service const* service,
    std::vector<time> relative_utc,
    std::unordered_set<unsigned> srv_traffic_days, merged_trips_idx trips_idx) {

  auto const& rel_utc_dep = relative_utc[section_idx * 2];
  auto const& rel_utc_arr = relative_utc[section_idx * 2 + 1];

  //  std::cout << "\nrel_utc_dep: " << rel_utc_dep;
  //  std::cout << "\nrel_utc_arr: " << rel_utc_arr;

  auto const day_offset = rel_utc_dep.day();
  uint16_t const utc_mam_dep =
      (rel_utc_dep - (day_offset * MINUTES_A_DAY)).ts();
  uint16_t const utc_mam_arr = utc_mam_dep + (rel_utc_arr - rel_utc_dep).ts();

  //  std::cout << "\nday_offset: " << day_offset;
  //  std::cout << "\nutc_mam_dep: " << utc_mam_dep;
  //  std::cout << "\nutc_mam_arr: " << utc_mam_arr;

  verify(utc_mam_dep <= utc_mam_arr, "departure must be before arrival");

  bitfield con_traffic_days;
  for (auto const& day : srv_traffic_days) {
    if (day_offset <= day) {
      // TODO: use (bitfield, offset) representation to save RAM
      con_traffic_days.set(day + day_offset);
    }
  }

  auto bitfield = get_or_create_bitfield(con_traffic_days);
  connection* full_con = get_full_connection(section_idx, service);
  return {bitfield, utc_mam_dep, utc_mam_arr, full_con, trips_idx};
}

std::vector<std::pair<std::vector<time>, std::unordered_set<unsigned>>>
graph_builder::service_times_to_utc(bitfield const& traffic_days, int start_idx,
                                    int end_idx, Service const* s) {
  std::vector<std::pair<std::vector<time>, std::unordered_set<unsigned>>>
      utc_times;
  std::vector<time> utc_service_times;
  utc_service_times.reserve(s->times()->size());
  for (int day_idx = start_idx; day_idx < end_idx; ++day_idx) {
    if (!traffic_days.test(day_idx)) {
      continue;
    }
    utc_service_times.clear();
    unsigned initial_day;
    int fix_offset = 0;
    for (unsigned i = 1; i < s->times()->size() - 1; ++i) {
      auto const& station = *sched_.stations_.at(
          stations_[s->route()->stations()->Get(i / 2)]->id_);

      auto local_time = s->times()->Get(i);
      auto const day_offset = local_time / MINUTES_A_DAY;
      local_time = local_time % MINUTES_A_DAY;
      int adj_day_idx = day_idx + day_offset + SCHEDULE_OFFSET_DAYS;

      bool const season = is_in_season(adj_day_idx, local_time, station.timez_);
      auto const season_offset = season ? station.timez_->season_.offset_
                                        : station.timez_->general_offset_;

      auto pre_utc = local_time - season_offset + fix_offset;
      if (pre_utc < 0) {
        pre_utc += 1440;
        adj_day_idx -= 1;
      }

      if (i == 1) {
        initial_day = adj_day_idx;
      }

      auto const abs_utc = time(adj_day_idx, pre_utc);

      auto const traffic_day_begin = time(initial_day, 0);
      auto const rel_utc = abs_utc - traffic_day_begin;

      auto const sort_ok = i == 1 || utc_service_times.back() <= rel_utc;
      auto const impossible_time =
          season && abs_utc < station.timez_->season_.begin_;
      if (!sort_ok || impossible_time) {
        /*        auto const last_local = time(s->times()->Get(i - 1));
                auto const curr_local = time(s->times()->Get(i));

                if (i != 1) {
                  auto const last_utc = utc_service_times.back();
                  LOG(debug) << last_local << " (local) -> " << last_utc <<
           "(utc)";
                }
                LOG(debug) << "impossible_time: " << impossible_time;
                LOG(debug) << "sort_ok: " << sort_ok;
                LOG(debug) << curr_local << " (local) -> " << rel_utc <<
           "(utc)";
                LOG(debug) << "norm_local: " << local_time;
                LOG(debug) << "day_offset: " << day_offset;
                LOG(debug) << "adj_day_idx: " << adj_day_idx;
                LOG(debug) << "season: " << season;
                LOG(debug) << "season_offset: " << season_offset;
                LOG(debug) << "fix_offset: " << fix_offset;
                LOG(debug) << "pre_utc: " << pre_utc;
                LOG(debug) << "abs_utc: " << abs_utc;
                LOG(debug) << "season_begin: " <<
           station.timez_->season_.begin_;

                LOG(warn) << "invalid time in schedule (data error)";*/
        verify(fix_offset < 60, "data error not recoverable (double adjust)");
        fix_offset += 60;
        --i;
        continue;
      }

      utc_service_times.emplace_back(rel_utc);
    }

    auto const& it = std::find_if(begin(utc_times), end(utc_times),
                                  [&utc_service_times](auto const& e) {
                                    return e.first == utc_service_times;
                                  });
    if (it == end(utc_times)) {
      std::pair<std::vector<motis::time>, std::unordered_set<unsigned>> entry =
          {utc_service_times, {initial_day}};
      utc_times.emplace_back(entry);
    } else {
      it->second.insert(initial_day);
    }
  }

  return utc_times;
}

void graph_builder::add_to_routes(std::vector<route_t>& alt_routes,
                                  std::vector<time> const& times,
                                  std::vector<light_connection> const& lcons) {
  for (auto& r : alt_routes) {
    if (r.add_service(lcons, times, sched_)) {
      return;
    }
  }

  alt_routes.emplace_back(route_t{lcons, times, sched_});
}

std::unique_ptr<route> graph_builder::create_route(Route const* r,
                                                   route_t const& lcons,
                                                   unsigned route_index) {
  auto const& stops = r->stations();
  auto const& in_allowed = r->in_allowed();
  auto const& out_allowed = r->out_allowed();
  auto route_sections = std::make_unique<route>();
  route_section last_route_section;

  verify(std::all_of(begin(lcons.lcons_), end(lcons.lcons_),
                     [&stops](auto const& lcon_string) {
                       return lcon_string.size() == stops->size() - 1;
                     }),
         "number of stops must match number of lcons");

  sched_.route_traffic_days_.emplace_back(bitfield{lcons.traffic_days_});
  if (!lcons.traffic_days_.any()) {
    LOG(info) << "route with no traffic days !?";
  }
  for (unsigned i = 0; i < stops->size() - 1; ++i) {
    auto from = i;
    auto to = i + 1;

    // get ith element of all lcon strings
    auto const section_lcons = utl::to_vec(
        lcons.lcons_, [&i](auto const& lcon_string) { return lcon_string[i]; });

    verify(section_lcons.size() == lcons.lcons_.size(),
           "number of connections on route segment must match number of "
           "services on route");

    route_sections->push_back(add_route_section(
        route_index, section_lcons,  //
        stops->Get(from), in_allowed->Get(from), out_allowed->Get(from),
        stops->Get(to), in_allowed->Get(to), out_allowed->Get(to),
        last_route_section.to_route_node_, nullptr,
        sched_.route_traffic_days_.size() - 1));
    last_route_section = route_sections->back();
  }

  return route_sections;
}

route_section graph_builder::add_route_section(
    int route_index, std::vector<light_connection> const& connections,
    Station const* from_stop, bool from_in_allowed, bool from_out_allowed,
    Station const* to_stop, bool to_in_allowed, bool to_out_allowed,
    node* from_route_node, node* to_route_node, size_t route_traffic_days) {
  route_section section;

  auto const from_station_node = stations_[from_stop];
  auto const to_station_node = stations_[to_stop];

  if (from_route_node != nullptr) {
    section.from_route_node_ = from_route_node;
  } else {
    section.from_route_node_ = build_route_node(
        route_index, next_node_id_++, from_station_node,
        sched_.stations_[from_station_node->id_]->transfer_time_,
        from_in_allowed, from_out_allowed);
  }

  if (to_route_node != nullptr) {
    section.to_route_node_ = to_route_node;
  } else {
    section.to_route_node_ =
        build_route_node(route_index, next_node_id_++, to_station_node,
                         sched_.stations_[to_station_node->id_]->transfer_time_,
                         to_in_allowed, to_out_allowed);
  }

  section.outgoing_route_edge_index_ = section.from_route_node_->edges_.size();

  verify(
      std::is_sorted(begin(connections), end(connections),
                     [](light_connection const& a, light_connection const& b) {
                       return a.d_time_ <= b.d_time_ && a.a_time_ <= b.a_time_;
                     }),
      "creating edge with lcons not strictly sorted");

  section.from_route_node_->edges_.push_back(
      make_route_edge(section.from_route_node_, section.to_route_node_,
                      connections, route_traffic_days));
  lcon_count_ += connections.size();

  return section;
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

std::string const* graph_builder::get_or_create_direction(
    Direction const* dir) {
  if (dir == nullptr) {
    return nullptr;
  } else if (dir->station()) {
    return &sched_.stations_[stations_[dir->station()]->id_]->name_;
  } else /* direction text */ {
    return utl::get_or_create(directions_, dir->text(), [&]() {
      sched_.directions_.emplace_back(
          std::make_unique<std::string>(dir->text()->str()));
      return sched_.directions_.back().get();
    });
  }
}

provider const* graph_builder::get_or_create_provider(Provider const* p) {
  if (p == nullptr) {
    return nullptr;
  } else {
    return utl::get_or_create(providers_, p, [&]() {
      sched_.providers_.emplace_back(std::make_unique<provider>(
          provider(p->short_name()->str(), p->long_name()->str(),
                   p->full_name()->str())));
      return sched_.providers_.back().get();
    });
  }
}

int graph_builder::get_or_create_category_index(Category const* c) {
  return utl::get_or_create(categories_, c, [&]() {
    int index = sched_.categories_.size();
    sched_.categories_.push_back(std::make_unique<category>(
        category(c->name()->str(), static_cast<uint8_t>(c->output_rule()))));
    return index;
  });
}

void graph_builder::write_trip_info(route& r) {
  auto const edges = utl::to_vec(begin(r), end(r), [](route_section& s) {
    return trip::route_edge(s.get_route_edge());
  });

  sched_.trip_edges_.emplace_back(new std::vector<trip::route_edge>(edges));
  auto edges_ptr = sched_.trip_edges_.back().get();

  auto& lcons = edges_ptr->front().get_edge()->m_.route_edge_.conns_;
  for (unsigned lcon_idx = 0; lcon_idx < lcons.size(); ++lcon_idx) {
    auto trp = sched_.merged_trips_[lcons[lcon_idx].trips_]->front();
    trp->edges_ = edges_ptr;
    trp->lcon_idx_ = lcon_idx;
  }
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

void graph_builder::sort_trips() {
  std::sort(begin(sched_.trips_), end(sched_.trips_));
}

size_t graph_builder::get_or_create_bitfield(bitfield const& bf) {
  sched_.bitfields_.emplace_back(bf);
  return sched_.bitfields_.size() - 1;
}

size_t graph_builder::get_or_create_bitfield(
    const String* serialized_bitfield) {
  auto const bits = deserialize_bitset<BIT_COUNT>(
      {serialized_bitfield->c_str(),
       static_cast<size_t>(serialized_bitfield->Length())});
  return get_or_create_bitfield(bits);
}

connection* graph_builder::get_full_connection(unsigned int section_idx,
                                               const Service* service) {
  auto& from = *sched_.stations_.at(
      stations_[service->route()->stations()->Get(section_idx)]->id_);
  auto& to = *sched_.stations_.at(
      stations_[service->route()->stations()->Get(section_idx + 1)]->id_);
  auto plfs = service->tracks();

  auto dep_platf = plfs ? plfs->Get(section_idx)->dep_tracks() : nullptr;
  auto arr_platf = plfs ? plfs->Get(section_idx + 1)->arr_tracks() : nullptr;

  return get_full_connection(service, from, to, dep_platf, arr_platf,
                             section_idx);
}

connection* graph_builder::get_full_connection(
    const Service* service, const station& from, const station& to,
    const flatbuffers64::Vector<flatbuffers64::Offset<Track>>* /*dep_platf*/,
    const flatbuffers64::Vector<flatbuffers64::Offset<Track>>* /*arr_platf*/,
    unsigned section_idx) {
  auto clasz_it = this->sched_.classes_.find(
      service->sections()->Get(section_idx)->category()->name()->str());
  this->con_.clasz_ =
      (clasz_it == end(this->sched_.classes_)) ? 9 : clasz_it->second;
  this->con_.price_ =
      get_distance(from, to) * get_price_per_km(this->con_.clasz_);
  this->con_.d_track_ = 0 /*this->get_or_create_track(0, dep_platf)*/;
  this->con_.a_track_ = 0 /*this->get_or_create_track(0, arr_platf)*/;
  this->con_.con_info_ =
      this->get_or_create_connection_info(service, section_idx, from_day_);

  return set_get_or_create(this->connections_, &this->con_, [&]() {
    this->sched_.full_connections_.emplace_back(
        std::make_unique<connection>(this->con_));
    return this->sched_.full_connections_.back().get();
  });
};

connection_info* graph_builder::get_or_create_connection_info(
    Section const* section, int /*dep_day_index*/) {
  con_info_.line_identifier_ =
      section->line_id() ? section->line_id()->str() : "";
  con_info_.train_nr_ = section->train_nr();
  con_info_.family_ = get_or_create_category_index(section->category());
  con_info_.dir_ = get_or_create_direction(section->direction());
  con_info_.provider_ = get_or_create_provider(section->provider());
  con_info_.merged_with_ = nullptr;
  // read_attributes(dep_day_index, section->attributes(),
  // con_info_.attributes_);

  return set_get_or_create(con_infos_, &con_info_, [&]() {
    sched_.connection_infos_.emplace_back(
        std::make_unique<connection_info>(con_info_));
    return sched_.connection_infos_.back().get();
  });
}

connection_info* graph_builder::get_or_create_connection_info(
    Service const* service, unsigned section_idx, int dep_day_index) {
  if (service == nullptr) {
    return nullptr;
  }

  return get_or_create_connection_info(service->sections()->Get(section_idx),
                                       dep_day_index);
}

void graph_builder::dedup_bitfields() {
  scoped_timer timer("bitfield deduplication");

  std::vector<size_t> map;
  auto& bfs = sched_.bitfields_;
  {
    scoped_timer timer("sort/unique");
    map = tracking_dedupe(
        bfs,  //
        [](auto const& a, auto const& b) { return a == b; },
        [&](auto const& a, auto const& b) { return bfs[a] < bfs[b]; });
    bfs.shrink_to_fit();
  }

  {
    scoped_timer timer("idx to ptr");
    for (auto& s : sched_.station_nodes_) {
      for (auto& route_node : s->get_route_nodes()) {
        for (auto& e : route_node->edges_) {
          if (e.type() != edge::ROUTE_EDGE) {
            continue;
          }
          for (auto& c : e.m_.route_edge_.conns_) {
            c.traffic_days_ = &sched_.bitfields_[map[c.bitfield_idx_]];
          }
        }
      }
    }
  }
}

schedule_ptr build_graph(Schedule const* serialized, time_t from, time_t to,
                         bool unique_check, bool apply_rules,
                         bool adjust_footpaths) {
  scoped_timer timer("building graph");

  schedule_ptr sched(new schedule());
  sched->classes_ = class_mapping();
  sched->schedule_begin_ = serialized->interval()->from();
  sched->schedule_end_ = serialized->interval()->to();

  LOG(info) << "schedule_begin: " << sched->schedule_begin_;

  if (serialized->name() != nullptr) {
    sched->name_ = serialized->name()->str();
  }

  graph_builder builder(*sched, serialized->interval(), from, to, apply_rules,
                        adjust_footpaths);
  LOG(info) << "loading day interval: [" << builder.from_day_ << ", "
            << builder.to_day_ << ")";

  builder.add_stations(serialized->stations());
  if (serialized->meta_stations() != nullptr) {
    builder.link_meta_stations(serialized->meta_stations());
  }

  builder.add_services(serialized->services());
  if (apply_rules) {
    scoped_timer timer("rule services");
    // verify(false, "not implemented!");
    LOG(error) << "apply rules enabled but not implemented";
  }

  builder.add_footpaths(serialized->footpaths());

  builder.connect_reverse();
  builder.sort_trips();
  builder.dedup_bitfields();  // DON'T TOUCH sched_.bitfields_ after this!

  sched->route_count_ = builder.next_route_index_;
  sched->node_count_ = builder.next_node_id_;
  sched->transfers_lower_bounds_fwd_ = build_interchange_graph(
      sched->station_nodes_, sched->route_count_, search_dir::FWD);
  sched->transfers_lower_bounds_bwd_ = build_interchange_graph(
      sched->station_nodes_, sched->route_count_, search_dir::BWD);
  sched->travel_time_lower_bounds_fwd_ =
      build_station_graph(sched->station_nodes_, search_dir::FWD);
  sched->travel_time_lower_bounds_bwd_ =
      build_station_graph(sched->station_nodes_, search_dir::BWD);
  sched->waiting_time_rules_ = load_waiting_time_rules(sched->categories_);
  sched->schedule_begin_ -= SCHEDULE_OFFSET_MINUTES * 60;

  LOG(info) << sched->stations_.size() << " stations";
  LOG(info) << sched->connection_infos_.size() << " connection infos";
  LOG(info) << builder.lcon_count_ << " light connections";
  LOG(info) << sched->bitfields_.size() << " lcon bitfields";
  LOG(info) << sched->route_traffic_days_.size() << " route bitfields";
  LOG(info) << builder.next_route_index_ << " routes";
  LOG(info) << sched->trip_mem_.size() << " trips";
  LOG(info) << serialized->services()->size()
            << " services (total not necessarily loaded)";

  /*
    std::ofstream fout("bitfields.bin");
    for (auto const& bf : bfs) {
      fout << *bf << "\n";
    }
    fout.close();
  */

  for (auto const& station : sched->station_nodes_) {
    for (auto const& route_node : station->get_route_nodes()) {
      for (auto& edge : route_node->edges_) {
        if (edge.empty()) {
          continue;
        }

        auto const bf_idx = edge.m_.route_edge_.bitfield_idx_;
        verify(bf_idx < sched->route_traffic_days_.size(),
               "invalid route traffic day bitfield index");
        edge.m_.route_edge_.traffic_days_ = &sched->route_traffic_days_[bf_idx];

        light_connection const* prev = nullptr;
        for (auto const& con : edge.m_.route_edge_.conns_) {
          verify(con.a_time_ >= con.d_time_,
                 "INVALID LCON: arrival before departure");
          if (prev != nullptr && prev->d_time_ >= con.d_time_) {
            std::cout << "\nprev.d_time: " << prev->d_time_
                      << "\tcurrent.dtime: " << con.d_time_;
            verify(false, "departures not sorted!");
          }
          prev = &con;
        }
      }
    }
  }

  return sched;
}

}  // namespace loader
}  // namespace motis
