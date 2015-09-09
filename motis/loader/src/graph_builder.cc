#include "motis/loader/graph_builder.h"

#include <functional>

#include "parser/cstr.h"

#include "motis/core/common/hash_map.h"
#include "motis/core/common/hash_set.h"
#include "motis/core/common/hash_helper.h"
#include "motis/core/common/logging.h"
#include "motis/core/schedule/price.h"

#include "motis/loader/wzr_loader.h"
#include "motis/loader/util.h"
#include "motis/loader/bitfield.h"
#include "motis/loader/classes.h"

using namespace motis::logging;
using namespace flatbuffers;

namespace motis {
namespace loader {

class graph_builder {
public:
  graph_builder(schedule& sched, Interval const* schedule_interval, time_t from,
                time_t to)
      : next_node_id_(-1),
        sched_(sched),
        first_day_((from - schedule_interval->from()) / (MINUTES_A_DAY * 60)),
        last_day_((to - schedule_interval->from()) / (MINUTES_A_DAY * 60) - 1) {
    connections_.set_empty_key(nullptr);
    con_infos_.set_empty_key(nullptr);
    bitfields_.set_empty_key(nullptr);
    stations_.set_empty_key(nullptr);
  }

  void add_stations(Vector<Offset<Station>> const* stations) {
    // Add dummy source station.
    auto dummy_source = make_unique<station>(0, 0.0, 0.0, 0, "-1", "DUMMY");
    sched_.eva_to_station.insert(
        std::make_pair(dummy_source->eva_nr, dummy_source.get()));
    sched_.stations.emplace_back(std::move(dummy_source));
    sched_.station_nodes.emplace_back(make_unique<station_node>(0));

    // Add dummy target stations.
    auto dummy_target = make_unique<station>(1, 0.0, 0.0, 0, "-2", "DUMMY");
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
      sched_.eva_to_station.insert(
          std::make_pair(input_station->id()->str(), s.get()));
      sched_.stations.emplace_back(std::move(s));
    }

    // First regular node id:
    // first id after station node ids
    next_node_id_ = sched_.stations.size();
  }

  void add_service(Service const* service) {
    auto const& sections = service->sections();

    auto route_nodes = get_or_create(
        routes_, service->route(), std::bind(&graph_builder::create_route, this,
                                             service->route(), routes_.size()));
    auto traffic_days = get_or_create_bitfield(service->traffic_days());

    for (unsigned section_idx = 0; section_idx < sections->size();
         ++section_idx) {
      add_service_section(
          &route_nodes[section_idx]->_edges[1],
          service->sections()->Get(section_idx),
          service->platforms()->Get(section_idx + 1)->arr_platforms(),
          service->platforms()->Get(section_idx)->dep_platforms(),
          service->times()->Get(section_idx * 2 + 1),
          service->times()->Get(section_idx * 2 + 2), traffic_days);
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
          edge._to->_incoming_edges.push_back(&edge);
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

  void add_service_section(edge* e, Section const* section,
                           Vector<Offset<Platform>> const* arr_platforms,
                           Vector<Offset<Platform>> const* dep_platforms,
                           int const dep_time, int const arr_time,
                           bitfield const& traffic_days) {
    assert(e->type() == edge::ROUTE_EDGE);

    // Departure station and arrival station.
    auto& from = *sched_.stations[e->_from->get_station()->_id];
    auto& to = *sched_.stations[e->_to->get_station()->_id];

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
          section->line_id() ? section->line_id()->str() : "";
      con_info_.train_nr = section->train_nr();
      con_info_.family = get_or_create_category_index(section->category());
      read_attributes(dep_day_index, section->attributes(),
                      con_info_.attributes);

      // Build full connection.
      con_.con_info = set_get_or_create(con_infos_, &con_info_, [&]() {
        sched_.connection_infos.emplace_back(
            make_unique<connection_info>(con_info_));
        return sched_.connection_infos.back().get();
      });

      con_.d_platform = get_or_create_platform(dep_day_index, dep_platforms);
      con_.a_platform = get_or_create_platform(arr_day_index, arr_platforms);

      auto clasz_it = sched_.classes.find(section->category()->name()->str());
      con_.clasz = (clasz_it == end(sched_.classes)) ? 9 : clasz_it->second;
      con_.price = get_distance(from, to) * get_price_per_km(con_.clasz);

      // Build light connection.
      int day_index = day - first_day_;
      e->_m._route_edge._conns.emplace_back(
          day_index * MINUTES_A_DAY + dep_time,
          day_index * MINUTES_A_DAY + arr_time,
          set_get_or_create(connections_, &con_, [&]() {
            sched_.full_connections.emplace_back(make_unique<connection>(con_));
            return sched_.full_connections.back().get();
          }));

      // Count events.
      ++from.dep_class_events[con_.clasz];
      ++to.arr_class_events[con_.clasz];
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

  int get_or_create_category_index(Category const* category) {
    return get_or_create(categories_, category->name(), [&]() {
      int index = sched_.category_names.size();
      sched_.category_names.push_back(category->name()->str());
      return index;
    });
  }

  int get_or_create_platform(int day,
                             Vector<Offset<Platform>> const* platforms) {
    static constexpr int NO_TRACK = 0;
    if (sched_.tracks.empty()) {
      sched_.tracks.push_back("");
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
      } else {
        sched_.route_index_to_first_route_node.push_back(route_node);
      }

      last_route_edge = &route_node->_edges.back();
      route_nodes.push_back(route_node);
    }
    return route_nodes;
  }

  std::map<String const*, int> categories_;
  std::map<Route const*, std::vector<node*>> routes_;
  std::map<std::string, int> tracks_;
  std::map<AttributeInfo const*, attribute*> attributes_;
  hash_map<Station const*, station_node*> stations_;
  hash_map<String const*, bitfield> bitfields_;
  hash_set<connection_info*,
           deep_ptr_hash<connection_info::hash, connection_info>,
           deep_ptr_eq<connection_info>> con_infos_;
  hash_set<connection*, deep_ptr_hash<connection::hash, connection>,
           deep_ptr_eq<connection>> connections_;
  unsigned next_node_id_;
  schedule& sched_;
  int first_day_, last_day_;

  connection_info con_info_;
  connection con_;
};

schedule_ptr build_graph(Schedule const* serialized, time_t from, time_t to) {
  scoped_timer timer("building graph");

  schedule_ptr sched(new schedule());
  sched->classes = class_mapping();
  sched->waiting_time_rules_ = load_waiting_time_rules(sched->category_names);
  sched->schedule_begin_ = from;
  sched->schedule_end_ = to;

  graph_builder builder(*sched.get(), serialized->interval(), from, to);
  builder.add_stations(serialized->stations());
  for (auto const& service : *serialized->services()) {
    builder.add_service(service);
  }
  builder.add_footpaths(serialized->footpaths());
  builder.connect_reverse();
  builder.sort_connections();

  sched->node_count = builder.node_count();
  sched->lower_bounds = constant_graph(sched->station_nodes);

  return sched;
}

}  // loader
}  // motis
