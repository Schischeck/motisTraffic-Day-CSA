#include "motis/loader/graph_builder.h"

#include <functional>

#include "motis/loader/wzr_loader.h"
#include "parser/cstr.h"

#include "motis/core/schedule/price.h"

#include "motis/core/common/logging.h"
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
        last_day_((to - schedule_interval->from()) / (MINUTES_A_DAY * 60)) {}

  void add_stations(Vector<Offset<Station>> const* stations) {
    for (unsigned i = 0; i < stations->size(); ++i) {
      auto const& input_station = stations->Get(i);

      // Create station node.
      auto node_ptr = make_unique<station_node>(i);
      stations_[input_station] = node_ptr.get();
      sched_.station_nodes.emplace_back(std::move(node_ptr));

      // Create station object.
      auto s = make_unique<station>();
      s->index = i;
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
    next_node_id_ = stations->size();
  }

  void add_service(Service const* service) {
    auto const& sections = service->sections();

    auto route_nodes = get_or_create(
        routes_, service->route(), std::bind(&graph_builder::create_route, this,
                                             service->route(), routes_.size()));

    auto serialized_traffic_days = service->traffic_days()->c_str();
    auto traffic_days = deserialize_bitset<512>(
        {serialized_traffic_days, service->traffic_days()->Length()});

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

  void transfer_connections() {
    for (auto& con_info : con_infos_) {
      sched_.connection_infos.emplace_back(std::move(con_info.second));
    }
    for (auto& con : connections_) {
      sched_.full_connections.emplace_back(std::move(con.second));
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

  int node_count() const { return next_node_id_; }

private:
  bitfield const& get_or_create_bitfield(String const* serialized_bitfield) {
    return get_or_create(bitfields_, serialized_bitfield, [&]() {
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
    auto const& from = *sched_.stations[e->_from->get_station()->_id];
    auto const& to = *sched_.stations[e->_to->get_station()->_id];

    // Expand traffic days.
    for (int day = 0; day < static_cast<int>(traffic_days.size()); ++day) {
      if (!traffic_days.test(day) || day < first_day_ || day > last_day_) {
        continue;
      }

      // Day indices for shifted bitfields (platforms, attributes)
      int dep_day_index = day + (dep_time / MINUTES_A_DAY);
      int arr_day_index = day + (arr_time / MINUTES_A_DAY);

      // Build connection info.
      connection_info con_info;
      if (section->line_id() != nullptr) {
        con_info.line_identifier = section->line_id()->str();
      }
      con_info.train_nr = section->train_nr();
      con_info.attributes =
          read_attributes(dep_day_index, section->attributes());
      con_info.family = get_or_create_category_index(section->category());

      // Build full connection.
      connection con;
      con.con_info = get_or_create(con_infos_, con_info, [&con_info]() {
        return make_unique<connection_info>(con_info);
      }).get();

      con.d_platform = get_or_create_platform(dep_day_index, dep_platforms);
      con.a_platform = get_or_create_platform(arr_day_index, arr_platforms);

      auto clasz_it = sched_.classes.find(section->category()->name()->str());
      con.clasz = (clasz_it == end(sched_.classes)) ? 9 : clasz_it->second;
      con.price = get_distance(from, to) * get_price_per_km(con.clasz);

      // Build light connection.
      int day_index = day - first_day_;
      e->_m._route_edge._conns.emplace_back(
          day_index * MINUTES_A_DAY * 60 + dep_time,
          day_index * MINUTES_A_DAY * 60 + arr_time,
          get_or_create(connections_, con, [&con]() {
            return make_unique<connection>(con);
          }).get());
    }
  }

  std::vector<attribute const*> read_attributes(
      int day, Vector<Offset<Attribute>> const* attributes) {
    std::vector<attribute const*> active_attributes;
    active_attributes.reserve(attributes->size());

    for (auto const& attr : *attributes) {
      if (!get_or_create_bitfield(attr->traffic_days()).test(day)) {
        continue;
      }
      active_attributes.push_back(get_or_create(
          attributes_, {attr->code()->c_str(), attr->code()->Length()}, [&]() {
            auto new_attr = make_unique<attribute>();
            new_attr->_code = attr->code()->str();
            new_attr->_str = attr->text()->str();
            sched_.attributes.emplace_back(std::move(new_attr));
            return sched_.attributes.back().get();
          }));
    }

    return active_attributes;
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
      auto const& station_node = stations_.at(from_stop);
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
        route_node->_edges.push_back(make_foot_edge(
            route_node, station_node,
            sched_.stations[station_id]->get_transfer_time(), true));
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
  std::map<String const*, bitfield> bitfields_;
  std::map<Route const*, std::vector<node*>> routes_;
  std::map<Station const*, station_node*> stations_;
  std::map<parser::cstr, attribute*> attributes_;
  std::map<connection_info, std::unique_ptr<connection_info>> con_infos_;
  std::map<connection, std::unique_ptr<connection>> connections_;
  std::map<std::string, int> tracks_;
  unsigned next_node_id_;
  schedule& sched_;
  int first_day_, last_day_;
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

  sched->node_count = builder.node_count();
  sched->lower_bounds = constant_graph(sched->station_nodes);
  builder.connect_reverse();
  builder.transfer_connections();

  return sched;
}

}  // loader
}  // motis
