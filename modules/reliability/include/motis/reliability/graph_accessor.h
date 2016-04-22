#pragma once

#include <algorithm>
#include <iostream>
#include <sstream>
#include <tuple>
#include <vector>

#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/schedule.h"

namespace motis {
namespace reliability {

namespace graph_accessor {

inline edge const* get_departing_route_edge(node const& route_node) {
  for (auto& edge : route_node.edges_) {
    if (!edge.empty()) {
      return &edge;
    }
  }
  return nullptr;
}

inline edge const* get_arriving_route_edge(node const& route_node) {
  for (auto& edge : route_node.incoming_edges_) {
    if (!edge->empty()) {
      return edge;
    }
  }
  return nullptr;
}

/**
 * returns the latest light-connection that arrives not later
 * than 'departure_time'.
 * Note: this function assumes that departing_light_conn is not the
 * first departure of the route.
 * @param route_node is the tail node of departing_light_conn
 */
inline light_connection const& get_previous_light_connection(
    array<light_connection> const& arriving_light_connections,
    motis::time const departure_time) {
  if (arriving_light_connections.size() == 1) {
    return arriving_light_connections.front();
  }
  auto ub = std::upper_bound(
      arriving_light_connections.begin(), arriving_light_connections.end(),
      departure_time,
      [](unsigned int const dep_time, light_connection const& lc) {
        return dep_time < lc.a_time_;
      });
  if (ub == arriving_light_connections.begin()) {
    return arriving_light_connections.front();
  } else if (ub == arriving_light_connections.end()) {
    return arriving_light_connections.back();
  }
  return *(--ub);
}

inline duration get_waiting_time(waiting_time_rules const& waiting_time_rules,
                                 uint32_t const family_feeder_train,
                                 uint32_t const family_connecting_train) {
  return (duration)waiting_time_rules.waiting_time(
      waiting_time_rules.waiting_time_category(family_connecting_train),
      waiting_time_rules.waiting_time_category(family_feeder_train));
}
inline duration get_waiting_time(
    waiting_time_rules const& waiting_time_rules,
    light_connection const& feeder_light_conn,
    light_connection const& connecting_light_conn) {
  return get_waiting_time(waiting_time_rules,
                          feeder_light_conn.full_con_->con_info_->family_,
                          connecting_light_conn.full_con_->con_info_->family_);
}

inline std::string train_name(
    light_connection const& conn,
    std::vector<std::unique_ptr<category>> const& categories) {
  std::stringstream sst;
  sst << categories[conn.full_con_->con_info_->family_]->name_
      << conn.full_con_->con_info_->train_nr_;
  return sst.str();
}

inline node const& get_first_route_node(node const& route_node) {
  node const* curr_node = &route_node;
  edge const* edge;
  while ((edge = get_arriving_route_edge(*curr_node)) != nullptr) {
    curr_node = edge->from_;
  }
  return *curr_node;
}

/* only for test-schedules: requires unique train numbers */
inline node const* get_first_route_node(schedule const& schedule,
                                        unsigned int const train_nr) {
  for (auto node : schedule.route_index_to_first_route_node_) {
    assert(graph_accessor::get_departing_route_edge(*node));
    assert(graph_accessor::get_departing_route_edge(*node)
               ->m_.route_edge_.conns_.size() > 0);
    if (graph_accessor::get_departing_route_edge(*node)
            ->m_.route_edge_.conns_[0]
            .full_con_->con_info_->train_nr_ == train_nr) {
      return node;
    }
  }
  return nullptr;
}

inline std::pair<light_connection const*, unsigned int> find_light_connection(
    edge const& route_edge, motis::time const event_time, bool is_departure,
    std::function<bool(connection_info const&)> check_connection_info) {
  auto lc_time = [is_departure](light_connection const& lc) -> unsigned int {
    return is_departure ? lc.d_time_ : lc.a_time_;
  };
  if (route_edge.empty()) {
    return std::make_pair(nullptr, 0);
  }
  auto it =
      std::lower_bound(std::begin(route_edge.m_.route_edge_.conns_),
                       std::end(route_edge.m_.route_edge_.conns_), event_time,
                       [&](light_connection const& a, unsigned int const b) {
                         return lc_time(a) < b;
                       });
  while (it != std::end(route_edge.m_.route_edge_.conns_) &&
         lc_time(*it) == event_time &&
         !check_connection_info(*it->full_con_->con_info_)) {
    ++it;
  }
  return (it == std::end(route_edge.m_.route_edge_.conns_) ||
          lc_time(*it) != event_time)
             ? std::make_pair(nullptr, 0)
             : std::make_pair(
                   it, it - std::begin(route_edge.m_.route_edge_.conns_));
}

inline std::pair<light_connection const*, unsigned int> find_light_connection(
    edge const& route_edge, motis::time const event_time, bool is_departure,
    unsigned int const family, unsigned int const train_nr,
    std::string const& line_identifier) {
  return find_light_connection(
      route_edge, event_time, is_departure, [&](connection_info const& ci) {
        return (train_nr == 0 || ci.train_nr_ == train_nr) &&
               ci.family_ == family && ci.line_identifier_ == line_identifier;
      });
}

inline duration walking_duration(node const& tail_station,
                                 node const& head_station) {
  node const* foot_node = nullptr;
  for (auto e : tail_station.edges_) {
    if (e.to_->is_foot_node()) {
      foot_node = e.to_;
      break;
    }
  }
  assert(foot_node);
  for (auto e : foot_node->edges_) {
    if (e.to_->id_ == head_station.id_) {
      return e.m_.foot_edge_.time_cost_;
    }
  }
  assert(false);
  return 0;
}

inline duration get_interchange_time(node const& arrival_node_feeder,
                                     node const& departure_node_departing_train,
                                     schedule const& schedule) {
  if (arrival_node_feeder.station_node_->id_ !=
      departure_node_departing_train.station_node_->id_) {
    return walking_duration(*arrival_node_feeder.station_node_,
                            *departure_node_departing_train.station_node_);
  }
  return schedule.stations_[departure_node_departing_train.station_node_->id_]
      ->transfer_time_;
}

/* note: category-names are not unique */
inline std::pair<bool, int> find_family(
    std::vector<std::unique_ptr<category>> const& categories,
    std::string const& category_name) {
  auto to_lower = [](std::string str) -> std::string {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
  };
  auto const cat_it =
      std::find_if(categories.begin(), categories.end(),
                   [&](std::unique_ptr<category> const& cat) {
                     return to_lower(cat->name_) == to_lower(category_name);
                   });

  if (cat_it == categories.end()) {
    return std::make_pair(false, 0);
  }
  return std::make_pair(true, cat_it - categories.begin());
}

inline void print_route_edge(edge const& route_edge, schedule const& schedule,
                             std::ostream& os) {
  os << "route-edge from "
     << schedule.stations_[route_edge.from_->get_station()->id_]->name_
     << "(route-node-id: " << route_edge.from_->id_ << ") to "
     << schedule.stations_[route_edge.to_->get_station()->id_]->name_
     << "(route-node-id: " << route_edge.to_->id_ << "):\n";
  unsigned int light_connection_idx = 0;
  for (auto const& light_connection : route_edge.m_.route_edge_.conns_) {
    os << schedule.stations_[route_edge.from_->station_node_->id_]->name_ << "/"
       << schedule.stations_[route_edge.from_->station_node_->id_]->eva_nr_
       << " " << format_time(light_connection.d_time_) << "--"
       << schedule.categories_[light_connection.full_con_->con_info_->family_]
              ->name_
       << light_connection.full_con_->con_info_->train_nr_ << ","
       << light_connection.full_con_->con_info_->line_identifier_ << "("
       << light_connection_idx++ << ")->"
       << format_time(light_connection.a_time_) << " "
       << schedule.stations_[route_edge.to_->station_node_->id_]->name_ << "/"
       << schedule.stations_[route_edge.to_->station_node_->id_]->eva_nr_
       << std::endl;
  }
}

inline void print_route(node const* const first_route_node,
                        schedule const& schedule, std::ostream& os) {
  node const* node = first_route_node;
  edge const* edge = nullptr;
  while ((edge = get_departing_route_edge(*node)) != nullptr) {
    print_route_edge(*edge, schedule, os);
    node = edge->to_;
  }
  os << std::endl;
}

}  // namespace graph_accessor
}  // namespace reliability
}  // namespace motis
