#include "motis/loader/graph_builder.h"

#include <functional>
#include <unordered_set>

#define RANGES_SUPPRESS_IOTA_WARNING
#include "range/v3/all.hpp"

#include "parser/cstr.h"

#include "motis/core/common/hash_map.h"
#include "motis/core/common/hash_set.h"
#include "motis/core/common/hash_helper.h"
#include "motis/core/common/logging.h"
#include "motis/core/schedule/price.h"
#include "motis/core/schedule/category.h"
#include "motis/core/schedule/provider.h"
#include "motis/core/schedule/timezone.h"

#include "motis/loader/wzr_loader.h"
#include "motis/loader/util.h"
#include "motis/loader/bitfield.h"
#include "motis/loader/classes.h"
#include "motis/loader/timezone_util.h"

using namespace motis::logging;
using namespace flatbuffers;
using namespace ranges;

namespace motis {
namespace loader {

class graph_builder {

public:
  graph_builder(schedule& sched, Interval const* schedule_interval, time_t from,
                time_t to, bool unique_check)
      : next_node_id_(-1),
        sched_(sched),
        first_day_((from - schedule_interval->from()) / (MINUTES_A_DAY * 60)),
        last_day_((to - schedule_interval->from()) / (MINUTES_A_DAY * 60) - 1),
        unique_check_(unique_check) {
    connections_.set_empty_key(nullptr);
    con_infos_.set_empty_key(nullptr);
    bitfields_.set_empty_key(nullptr);
    stations_.set_empty_key(nullptr);
  }

  void add_stations(Vector<Offset<Station>> const* stations) {
    // Add dummy source station.
    auto dummy_source =
        make_unique<station>(0, 0.0, 0.0, 0, "-1", "DUMMY", nullptr);
    sched_.eva_to_station.insert(
        std::make_pair(dummy_source->eva_nr, dummy_source.get()));
    sched_.stations.emplace_back(std::move(dummy_source));
    sched_.station_nodes.emplace_back(make_unique<station_node>(0));

    // Add dummy target stations.
    auto dummy_target =
        make_unique<station>(1, 0.0, 0.0, 0, "-2", "DUMMY", nullptr);
    sched_.eva_to_station.insert(
        std::make_pair(dummy_target->eva_nr, dummy_target.get()));
    sched_.stations.emplace_back(std::move(dummy_target));
    sched_.station_nodes.emplace_back(make_unique<station_node>(1));

    // Add schedule stations.
    for (unsigned i = 0; i < stations->size(); ++i) {
      auto const& input_station = stations->Get(i);
      auto const station_index = i + 2;

      // Create station node.
      auto node_ptr = make_unique<station_node>(station_index);
      stations_[input_station] = node_ptr.get();
      sched_.station_nodes.emplace_back(std::move(node_ptr));

      // Create station object.
      auto s = make_unique<station>();
      s->index = station_index;
      s->name = input_station->name()->str();
      s->width = input_station->lat();
      s->length = input_station->lng();
      s->eva_nr = input_station->id()->str();
      s->transfer_time = input_station->interchange_time();
      // TODO deprecation warning: timezone is expected to be non null!
      s->timez = input_station->timezone()
                     ? get_or_create_timezone(input_station->timezone())
                     : nullptr;
      sched_.eva_to_station.insert(
          std::make_pair(input_station->id()->str(), s.get()));
      sched_.stations.emplace_back(std::move(s));

      // Store DS100.
      if (input_station->external_ids()) {
        for (auto const& ds100 : *input_station->external_ids()) {
          sched_.ds100_to_station.insert(std::make_pair(ds100->str(), s.get()));
        }
      }
    }

    // First regular node id:
    // first id after station node ids
    next_node_id_ = sched_.stations.size();
  }

  timezone const* get_or_create_timezone(Timezone const* input_timez) {
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
      sched_.timezones.emplace_back(new timezone(tz));
      return sched_.timezones.back().get();
    });
  }

  bool is_unique_service(Service const* service, bitfield const& traffic_days,
                         std::vector<node*> const& route_nodes) const {
    auto const& sections = service->sections();
    for (unsigned section_idx = 0; section_idx < sections->size();
         ++section_idx) {
      unsigned train_nr = sections->Get(section_idx)->train_nr();
      auto base_d_time = service->times()->Get(section_idx * 2 + 1);
      auto base_a_time = service->times()->Get(section_idx * 2 + 2);
      auto route_node = route_nodes[section_idx];
      auto const& route_edge = route_node->_edges[1];
      auto from_station = route_edge._from->get_station();
      auto to_station = route_edge._to->get_station();
      for (int day = first_day_; day <= last_day_; ++day) {
        if (!traffic_days.test(day)) {
          continue;
        }

        int time_offset = (day - first_day_) * MINUTES_A_DAY;
        auto d_time = time_offset + base_d_time;
        auto a_time = time_offset + base_a_time;
        // check for other connections in the same route edge
        // that have the same departure or arrival time
        for (auto const& lc : route_edge._m._route_edge._conns) {
          if (lc.d_time == d_time || lc.a_time == a_time) {
            return false;
          }
        }
        // check if other routes contain an event with the same time and
        // train number
        for (auto const& e : from_station->_edges) {
          if (e._to->is_route_node() && e._to->_edges.size() > 1) {
            auto const& other_route_edge = e._to->_edges[1];
            for (auto const& lc : other_route_edge._m._route_edge._conns) {
              if (lc._full_con->con_info->train_nr == train_nr &&
                  lc.d_time == d_time) {

                return false;
              }
            }
          }
        }
        for (auto const& e : to_station->_edges) {
          if (e._to->is_route_node()) {
            if (e._to->_incoming_edges.size() == 1) {
              auto const& other_route_edge = e._to->_incoming_edges[0];
              for (auto const& lc : other_route_edge->_m._route_edge._conns) {
                if (lc._full_con->con_info->train_nr == train_nr &&
                    lc.a_time == a_time) {
                  return false;
                }
              }
            }
          }
        }
      }
    }
    return true;
  }

  void add_service(Service const* service) {
    auto const& sections = service->sections();

    auto traffic_days = get_or_create_bitfield(service->traffic_days());

    if (!accumulate(view::closed_ints(first_day_, last_day_), false,
                    [&traffic_days](bool acc, int day) {
                      return acc || traffic_days.test(day);
                    })) {
      return;
    }

    auto route_nodes = get_or_create(
        routes_, service->route(), std::bind(&graph_builder::create_route, this,
                                             service->route(), routes_.size()));
    if (unique_check_ &&
        !is_unique_service(service, traffic_days, route_nodes)) {
      return;
    }

    std::unordered_set<uint32_t> train_nrs;
    for (unsigned section_idx = 0; section_idx < sections->size();
         ++section_idx) {
      add_service_section(
          &route_nodes[section_idx]->_edges[1],
          service->sections()->Get(section_idx),
          service->platforms()
              ? service->platforms()->Get(section_idx + 1)->arr_platforms()
              : nullptr,
          service->platforms()
              ? service->platforms()->Get(section_idx)->dep_platforms()
              : nullptr,
          service->times()->Get(section_idx * 2 + 1),
          service->times()->Get(section_idx * 2 + 2), traffic_days,
          service->origin());
      train_nrs.insert(service->sections()->Get(section_idx)->train_nr());
    }
    int32_t route_id = route_nodes[0]->_route;
    for (uint32_t train_nr : train_nrs) {
      auto& routes = sched_.train_nr_to_routes[train_nr];
      if (std::find(std::begin(routes), std::end(routes), route_id) ==
          std::end(routes)) {
        routes.push_back(route_id);
      }
    }
  }

  void add_footpaths(Vector<Offset<Footpath>> const* footpaths) {
    for (auto const& footpath : *footpaths) {
      auto from = stations_[footpath->from()];
      auto to = stations_[footpath->to()];
      next_node_id_ = from->add_foot_edge(
          next_node_id_, make_foot_edge(from, to, footpath->duration()));
    }
  }

  void connect_reverse() {
    for (auto& station_node : sched_.station_nodes) {
      for (auto& station_edge : station_node->_edges) {
        station_edge._to->_incoming_edges.push_back(&station_edge);
        for (auto& edge : station_edge._to->_edges) {
          if (edge.type() != edge::ROUTE_EDGE) {
            edge._to->_incoming_edges.push_back(&edge);
          }
        }
      }
    }
  }

  void sort_connections() {
    for (auto& station_node : sched_.station_nodes) {
      for (auto& station_edge : station_node->_edges) {
        for (auto& edge : station_edge._to->_edges) {
          if (edge.empty()) {
            continue;
          }
          std::sort(begin(edge._m._route_edge._conns),
                    end(edge._m._route_edge._conns));
        }
      }
    }
  }

  int node_count() const { return next_node_id_; }

private:
  bitfield const& get_or_create_bitfield(String const* serialized_bitfield) {
    return map_get_or_create(bitfields_, serialized_bitfield, [&]() {
      return deserialize_bitset<BIT_COUNT>(
          {serialized_bitfield->c_str(), serialized_bitfield->Length()});
    });
  }

  void add_service_section(edge* curr_route_edge, Section const* curr_section,
                           Vector<Offset<Platform>> const* arr_platforms,
                           Vector<Offset<Platform>> const* dep_platforms,
                           int const dep_time, int const arr_time,
                           bitfield const& traffic_days, String const* origin) {
    assert(curr_route_edge->type() == edge::ROUTE_EDGE);

    // Departure station and arrival station.
    auto& from = *sched_.stations[curr_route_edge->_from->get_station()->_id];
    auto& to = *sched_.stations[curr_route_edge->_to->get_station()->_id];

    // Expand traffic days.
    for (int day = first_day_; day <= last_day_; ++day) {
      if (!traffic_days.test(day)) {
        continue;
      }

      // Day indices for shifted bitfields (platforms, attributes)
      int dep_day_index = day + (dep_time / MINUTES_A_DAY);
      int arr_day_index = day + (arr_time / MINUTES_A_DAY);

      // Build connection info.
      con_info_.line_identifier =
          curr_section->line_id() ? curr_section->line_id()->str() : "";
      con_info_.train_nr = curr_section->train_nr();
      con_info_.family = get_or_create_category_index(curr_section->category());
      con_info_.dir_ = get_or_create_direction(curr_section->direction());
      con_info_.provider_ = get_or_create_provider(curr_section->provider());
      read_attributes(dep_day_index, curr_section->attributes(),
                      con_info_.attributes);

      // Build full connection.
      con_.con_info = set_get_or_create(con_infos_, &con_info_, [&]() {
        sched_.connection_infos.emplace_back(
            make_unique<connection_info>(con_info_));
        return sched_.connection_infos.back().get();
      });

      con_.d_platform = get_or_create_platform(dep_day_index, dep_platforms);
      con_.a_platform = get_or_create_platform(arr_day_index, arr_platforms);

      auto clasz_it =
          sched_.classes.find(curr_section->category()->name()->str());
      con_.clasz = (clasz_it == end(sched_.classes)) ? 9 : clasz_it->second;
      con_.price = get_distance(from, to) * get_price_per_km(con_.clasz);

      // Build light connection.
      auto const* dep_st =
          sched_.stations.at(curr_route_edge->_from->get_station()->_id).get();
      auto const* arr_st =
          sched_.stations.at(curr_route_edge->_to->get_station()->_id).get();

      auto const dep_motis_time =
          compute_event_time(day, dep_time, dep_st->timez);
      auto const arr_motis_time =
          compute_event_time(day, arr_time, arr_st->timez);

      //      validate_events(day, dep_st, arr_st, dep_motis_time,
      //      arr_motis_time,
      //                      dep_time, arr_time, origin);

      curr_route_edge->_m._route_edge._conns.emplace_back(
          dep_motis_time, arr_motis_time,
          set_get_or_create(connections_, &con_, [&]() {
            sched_.full_connections.emplace_back(make_unique<connection>(con_));
            return sched_.full_connections.back().get();
          }));

      // Count events.
      ++from.dep_class_events[con_.clasz];
      ++to.arr_class_events[con_.clasz];
    }
  }

  // TODO dep_tz, arr_tz == nullptr is deprecated.
  // Each station should have a timezone
  time compute_event_time(int day, time local_time, timezone const* tz) {
    return tz ? tz->to_motis_time(day - first_day_, local_time)
              : (day - first_day_ + 1) * MINUTES_A_DAY + local_time;
  }

  // TODO dep_tz, arr_tz == nullptr is deprecated.
  // Each station should have a timezone
  void validate_events(int day, station const* dep_st, station const* arr_st,
                       time dep_motis_time, time arr_motis_time,
                       int dep_local_time, int arr_local_time,
                       String const* origin) {
    auto const is_invalid_dep_event =
        dep_st->timez && dep_st->timez->is_invalid_time(dep_motis_time);
    auto const is_invalid_arr_event =
        arr_st->timez && arr_st->timez->is_invalid_time(arr_motis_time);
    auto const is_negative_edge = dep_motis_time > arr_motis_time;

    if (is_invalid_dep_event || is_invalid_arr_event || is_negative_edge) {
      LOG(emrg) << "[" << origin->c_str() << "]";
    }
    if (is_invalid_dep_event) {
      LOG(emrg) << "invalid departure time for (loader) day_idx: " << day;
    }
    if (is_invalid_arr_event) {
      LOG(emrg) << "invalid arrival time for (loader) day_idx: " << day;
    }
    if (is_negative_edge) {
      LOG(emrg) << "negative edge at section (" << dep_st->eva_nr << ","
                << arr_st->eva_nr << ")";
      LOG(emrg) << dep_local_time << "--local_time-->" << arr_local_time;
      LOG(emrg) << dep_motis_time << "--motis_time-->" << arr_motis_time;
    }
  }

  void read_attributes(int day, Vector<Offset<Attribute>> const* attributes,
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
            new_attr->_code = attribute_info->code()->str();
            new_attr->_str = attribute_info->text()->str();
            sched_.attributes.emplace_back(std::move(new_attr));
            return sched_.attributes.back().get();
          }));
    }
  }

  std::string const* get_or_create_direction(Direction const* dir) {
    if (dir == nullptr) {
      return nullptr;
    } else if (dir->station()) {
      return &sched_.stations[stations_[dir->station()]->_id]->name;
    } else /* direction text */ {
      return get_or_create(directions_, dir->text(), [&]() {
        sched_.directions.emplace_back(
            make_unique<std::string>(dir->text()->str()));
        return sched_.directions.back().get();
      });
    }
  }

  provider const* get_or_create_provider(Provider const* p) {
    if (p == nullptr) {
      return nullptr;
    } else {
      return get_or_create(providers_, p, [&]() {
        sched_.providers.emplace_back(make_unique<provider>(
            provider(p->short_name()->str(), p->long_name()->str(),
                     p->full_name()->str())));
        return sched_.providers.back().get();
      });
    }
  }

  int get_or_create_category_index(Category const* c) {
    return get_or_create(categories_, c, [&]() {
      int index = sched_.categories.size();
      sched_.categories.push_back(make_unique<category>(
          category(c->name()->str(), static_cast<uint8_t>(c->output_rule()))));
      return index;
    });
  }

  int get_or_create_platform(int day,
                             Vector<Offset<Platform>> const* platforms) {
    static constexpr int NO_TRACK = 0;
    if (sched_.tracks.empty()) {
      sched_.tracks.push_back("");
    }

    if (platforms == nullptr) {
      return NO_TRACK;
    }

    auto track_it = std::find_if(
        std::begin(*platforms), std::end(*platforms),
        [&](Platform const* track) {
          return get_or_create_bitfield(track->bitfield()).test(day);
        });
    if (track_it == std::end(*platforms)) {
      return NO_TRACK;
    } else {
      auto name = track_it->name()->str();
      return get_or_create(tracks_, name, [&]() {
        int index = sched_.tracks.size();
        sched_.tracks.push_back(name);
        return index;
      });
    }
  }

  std::vector<node*> create_route(Route const* route, int route_index) {
    auto const& stops = route->stations();
    auto const& in_allowed = route->in_allowed();
    auto const& out_allowed = route->out_allowed();
    std::vector<node*> route_nodes;
    edge* last_route_edge = nullptr;
    for (unsigned stop_idx = 0; stop_idx < stops->size(); ++stop_idx) {
      auto const& from_stop = stops->Get(stop_idx);
      auto const& station_node = stations_[from_stop];
      auto station_id = station_node->_id;
      auto route_node = new node(station_node, next_node_id_++);
      route_node->_route = route_index;

      // Connect the new route node with the corresponding station node:
      // route -> station: edge cost = change time, interchange count
      // station -> route: free
      if (in_allowed->Get(stop_idx) == 0) {
        station_node->_edges.push_back(
            make_invalid_edge(station_node, route_node));
      } else {
        station_node->_edges.push_back(
            make_foot_edge(station_node, route_node));
      }
      if (out_allowed->Get(stop_idx) == 0) {
        route_node->_edges.push_back(
            make_invalid_edge(route_node, station_node));
      } else {
        route_node->_edges.push_back(
            make_foot_edge(route_node, station_node,
                           sched_.stations[station_id]->transfer_time, true));
      }

      // Connect route nodes with route edges.
      if (stop_idx != stops->size() - 1) {
        route_node->_edges.push_back(make_route_edge(route_node, nullptr, {}));
      }
      if (last_route_edge != nullptr) {
        last_route_edge->_to = route_node;
        route_node->_incoming_edges.push_back(last_route_edge);
      } else {
        sched_.route_index_to_first_route_node.push_back(route_node);
      }

      last_route_edge = &route_node->_edges.back();
      route_nodes.push_back(route_node);
    }
    return route_nodes;
  }

  std::map<Category const*, int> categories_;
  std::map<Route const*, std::vector<node*>> routes_;
  std::map<std::string, int> tracks_;
  std::map<AttributeInfo const*, attribute*> attributes_;
  std::map<String const*, std::string const*> directions_;
  std::map<Provider const*, provider const*> providers_;
  hash_map<Station const*, station_node*> stations_;
  std::map<Timezone const*, timezone const*> timezones_;
  hash_map<String const*, bitfield> bitfields_;
  hash_set<connection_info*,
           deep_ptr_hash<connection_info::hash, connection_info>,
           deep_ptr_eq<connection_info>> con_infos_;
  hash_set<connection*, deep_ptr_hash<connection::hash, connection>,
           deep_ptr_eq<connection>> connections_;
  unsigned next_node_id_;
  schedule& sched_;
  int first_day_, last_day_;
  bool unique_check_;

  connection_info con_info_;
  connection con_;
};

schedule_ptr build_graph(Schedule const* serialized, time_t from, time_t to,
                         bool unique_check) {
  scoped_timer timer("building graph");

  schedule_ptr sched(new schedule());
  sched->classes = class_mapping();
  sched->schedule_begin_ = from;
  sched->schedule_end_ = to;

  graph_builder builder(*sched.get(), serialized->interval(), from, to,
                        unique_check);
  builder.add_stations(serialized->stations());
  for (auto const& service : *serialized->services()) {
    builder.add_service(service);
  }
  builder.add_footpaths(serialized->footpaths());
  builder.connect_reverse();
  builder.sort_connections();

  sched->node_count = builder.node_count();
  sched->lower_bounds = constant_graph(sched->station_nodes);
  sched->waiting_time_rules_ = load_waiting_time_rules(sched->categories);

  sched->schedule_begin_ -= SCHEDULE_OFFSET;
  return sched;
}

}  // loader
}  // motis
