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

  inline td::Edge* get_arriving_route_edge(td::Node& route_node) {
    for (auto& edge : route_node._incomingEdges) {
      if (!edge->empty()) {
        return edge;
      }
    }
    return nullptr;
  }

  inline td::LightConnection& get_previous_light_connection(td::Node& route_node,
      td::LightConnection& departing_light_connection) {
    auto arriving_route_edge = get_arriving_route_edge(route_node);

    auto& all_connections = arriving_route_edge->_m._routeEdge._conns;
    unsigned int pos = 0;
    while (pos+1 < all_connections.size()
        && all_connections[pos+1].aTime <= departing_light_connection.dTime)
      pos++;

    return all_connections[pos];
  }

}

}  // namespace reliability
}  // namespace motis
