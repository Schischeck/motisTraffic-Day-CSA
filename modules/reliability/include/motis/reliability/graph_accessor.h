#pragma once

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

inline edge const* const get_departing_route_edge(node const& route_node) {
  for (auto& edge : route_node._edges) {
    if (!edge.empty()) {
      return &edge;
    }
  }
  return nullptr;
}

inline edge const* const get_arriving_route_edge(node const& route_node) {
  for (auto& edge : route_node._incoming_edges) {
    if (!edge->empty()) {
      return edge;
    }
  }
  return nullptr;
}

/**
 * returns the light-connection that belongs to the train of
 * 'departing_light_conn', arrives at the departure station of
 * 'departing_light_conn', and has an arrival time next to the departure time of
 * 'departing_light_conn'.
 * This function also returns the position of the light-connection's arrival
 * distribution stored in 'route_node'.
 */
inline std::pair<light_connection const* const, unsigned int>
get_previous_light_connection(node const& route_node,
                              light_connection const& departing_light_conn) {
  auto arriving_route_edge = get_arriving_route_edge(route_node);

  auto& all_connections = arriving_route_edge->_m._route_edge._conns;
  unsigned int pos = 0;
  while (pos + 1 < all_connections.size() &&
         all_connections[pos + 1].a_time <= departing_light_conn.d_time) {
    ++pos;
  }

  return std::make_pair(&all_connections[pos], pos);
}

/* returns success, begin_time, end_time */
inline std::tuple<bool, time, time> get_feeder_time_interval(
    time const departure_time, duration const change_time,
    duration const feeder_threshold) {
  time const begin_time =
      (departure_time <= feeder_threshold ? 0
                                          : departure_time - feeder_threshold);
  time const end_time = departure_time - change_time;
  bool const success =
      (begin_time <= end_time && departure_time >= change_time);

  return std::make_tuple(success, begin_time, end_time);
}

#define FEEDER_THRESHOLD 30 /* XXX */

struct feeder_info {
  feeder_info(node const& route_node, light_connection const& light_conn,
              unsigned int const light_conn_idx)
      : route_node_(route_node),
        light_conn_(light_conn),
        light_conn_idx_(light_conn_idx) {}
  node const& route_node_;
  light_connection const& light_conn_;
  unsigned int const light_conn_idx_;
};
inline std::vector<std::unique_ptr<feeder_info> > get_all_potential_feeders(
    node const& route_node, light_connection const& departing_light_conn) {
  std::vector<std::unique_ptr<feeder_info> > feeders;

  for (auto const in_edge : route_node._station_node->_incoming_edges) {
    // ignore transfer edge to route_node itself
    if (in_edge->_from->_id == route_node._id) {
      continue;
    }

    bool success;
    time time_begin, time_end;
    std::tie(success, time_begin, time_end) = get_feeder_time_interval(
        departing_light_conn.d_time, in_edge->_m._foot_edge._time_cost,
        FEEDER_THRESHOLD);
    if (!success) {
      continue;
    }

    auto& feeder_route_node = *in_edge->_from._ptr;
    auto const feeder_route_edge = get_arriving_route_edge(feeder_route_node);
    if (feeder_route_edge == nullptr) continue;

    auto& all_connections = feeder_route_edge->_m._route_edge._conns;
    for (unsigned int i = 0; i < all_connections.size(); i++) {
      if (all_connections[i].a_time >= time_begin &&
          all_connections[i].a_time <= time_end) {
        feeders.emplace_back(
            new feeder_info(feeder_route_node, all_connections[i], i));
      }
    }
  }

  return feeders;
}

inline duration get_waiting_time(
    waiting_time_rules const& waiting_time_rules,
    light_connection const& feeder_light_conn,
    light_connection const& connecting_light_conn) {
  return (duration)waiting_time_rules.waiting_time(
      waiting_time_rules.waiting_time_category(
          connecting_light_conn._full_con->con_info->family),
      waiting_time_rules.waiting_time_category(
          feeder_light_conn._full_con->con_info->family));
}

inline std::string train_name(light_connection const& conn,
                              std::vector<std::string> const& category_names) {
  std::stringstream sst;
  sst << category_names[conn._full_con->con_info->family]
      << conn._full_con->con_info->train_nr;
  return sst.str();
}

inline node const& get_first_route_node(node const& route_node) {
  node const* curr_node = &route_node;
  edge const* edge;
  while ((edge = get_arriving_route_edge(*curr_node)) != nullptr) {
    curr_node = edge->_from;
  }
  return *curr_node;
}

inline void print_route(node const* const first_route_node,
                        schedule const& schedule, std::ostream& os) {
  node const* node = first_route_node;
  edge const* edge = nullptr;
  while ((edge = get_departing_route_edge(*node)) != nullptr) {
    os << "route-edge from "
       << schedule.stations[node->get_station()->_id]->name << " to "
       << schedule.stations[edge->_to->get_station()->_id]->name << ":\n";
    unsigned int light_connection_idx = 0;
    for (auto const& light_connection : edge->_m._route_edge._conns) {
      os << schedule.stations[edge->_from->_station_node->_id]->name << "("
         << edge->_from->_id << ")"
         << " " << format_time(light_connection.d_time) << "--"
         << schedule
                .category_names[light_connection._full_con->con_info->family]
         << light_connection._full_con->con_info->train_nr << "("
         << light_connection_idx++ << ")->"
         << format_time(light_connection.a_time) << " "
         << schedule.stations[edge->_to->_station_node->_id]->name << "("
         << edge->_to->_id << ")"
         << " " << &light_connection << "\n";
    }
    node = edge->_to;
  }
  os << std::endl;
}

}  // namespace graph_accessor
}  // namespace reliability
}  // namespace motis
