#pragma once

#include "motis/core/schedule/Nodes.h"
#include "motis/core/schedule/Edges.h"

namespace motis {
namespace reliability {

namespace graph_accessor {

inline td::Edge* get_departing_route_edge(td::Node& route_node) {
  for (auto& edge : route_node._edges) {
    if (!edge.empty()) {
      return &edge;
    }
  }
  return nullptr;
}

inline td::Edge const* get_arriving_route_edge(td::Node const& route_node) {
  for (auto& edge : route_node._incomingEdges) {
    if (!edge->empty()) {
      return edge;
    }
  }
  return nullptr;
}

inline std::pair<td::LightConnection const*, unsigned int>
get_previous_light_connection(td::Node const& route_node,
                              td::LightConnection const& departing_light_conn) {
  auto arriving_route_edge = get_arriving_route_edge(route_node);

  auto& all_connections = arriving_route_edge->_m._routeEdge._conns;
  unsigned int pos = 0;
  while (pos + 1 < all_connections.size() &&
         all_connections[pos + 1].aTime <= departing_light_conn.dTime) {
    ++pos;
  }

  return std::make_pair(&all_connections[pos], pos);
}

/* feeder-route-node, feeder-light-connection, feeder-distribution-position */
typedef std::tuple<td::Node const*, td::LightConnection const*, unsigned int>
    feeder_info;
inline std::vector<feeder_info> get_all_potential_feeders(
    td::Node& route_node, td::LightConnection const& departing_light_conn) {
  std::vector<feeder_info> feeders;

  for (auto const in_edge : route_node._stationNode->_incomingEdges) {
    if (!in_edge->_to->isRouteNode() || in_edge->_from->_id == route_node._id)
      continue;

    td::Time const time_end =
        departing_light_conn.dTime - in_edge->_m._footEdge._timeCost;
    td::Time const time_begin = time_end - 30;  // XXX 30 minutes

    auto& feeder_route_node = *in_edge->_from._ptr;
    auto const feeder_route_edge = get_arriving_route_edge(feeder_route_node);
    if (feeder_route_edge == nullptr) continue;

    auto& all_connections = feeder_route_edge->_m._routeEdge._conns;

    for (unsigned int i = 0; i < all_connections.size(); i++) {
      if (all_connections[i].aTime >= time_begin &&
          all_connections[i].aTime <= time_end) {
        feeders.emplace_back(&feeder_route_node, &all_connections[i], i);
      }
    }
  }

  return feeders;
}
}

}  // namespace reliability
}  // namespace motis
