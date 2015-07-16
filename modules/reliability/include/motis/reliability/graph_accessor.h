#pragma once

#include <tuple>
#include <vector>

#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/edges.h"

namespace motis {
namespace reliability {

namespace graph_accessor {

inline edge* get_departing_route_edge(node& route_node) {
  for (auto& edge : route_node._edges) {
    if (!edge.empty()) {
      return &edge;
    }
  }
  return nullptr;
}

inline edge const* get_arriving_route_edge(node const& route_node) {
  for (auto& edge : route_node._incoming_edges) {
    if (!edge->empty()) {
      return edge;
    }
  }
  return nullptr;
}

inline std::pair<light_connection const*, unsigned int>
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

/* feeder-route-node, feeder-light-connection, feeder-distribution-position */
typedef std::tuple<node const*, light_connection const*, unsigned int>
    feeder_info;
inline std::vector<feeder_info> get_all_potential_feeders(
    node const& route_node, light_connection const& departing_light_conn) {
  std::vector<feeder_info> feeders;

  for (auto const in_edge : route_node._station_node->_incoming_edges) {
    if (!in_edge->_to->is_route_node() || in_edge->_from->_id == route_node._id)
      continue;

    time const time_end =
        departing_light_conn.d_time - in_edge->_m._foot_edge._time_cost;
    time const time_begin = time_end - 30;  // XXX 30 minutes

    auto& feeder_route_node = *in_edge->_from._ptr;
    auto const feeder_route_edge = get_arriving_route_edge(feeder_route_node);
    if (feeder_route_edge == nullptr) continue;

    auto& all_connections = feeder_route_edge->_m._route_edge._conns;

    for (unsigned int i = 0; i < all_connections.size(); i++) {
      if (all_connections[i].a_time >= time_begin &&
          all_connections[i].a_time <= time_end) {
        feeders.emplace_back(&feeder_route_node, &all_connections[i], i);
      }
    }
  }

  return feeders;
}

inline std::pair<unsigned int, unsigned int> get_departure_distribution_indices(
    node const& arrival_route_node,
    light_connection const& arriving_light_conn) {
  auto const& route_edge = get_arriving_route_edge(arrival_route_node);
  // find index of 'arriving_light_conn'
  auto const& all_connections = route_edge->_m._route_edge._conns;
  unsigned int pos = 0;
  while (pos < all_connections.size() &&
         (all_connections[pos].d_time != arriving_light_conn.d_time
             || all_connections[pos].a_time != arriving_light_conn.a_time)) {
    ++pos;
  }
  return std::make_pair(route_edge->_from->_id, pos);
}

}  // namespace graph_accessor
}  // namespace reliability
}  // namespace motis
