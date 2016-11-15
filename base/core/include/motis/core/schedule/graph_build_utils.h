#pragma once

#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/nodes.h"

namespace motis {

inline node* build_route_node(int route_index, int node_id,
                              station_node* station_node, int transfer_time,
                              bool in_allowed, bool out_allowed) {
  auto route_node = new node(station_node, node_id, route_index);

  if (!in_allowed) {
    station_node->edges_.push_back(make_invalid_edge(station_node, route_node));
  } else {
    station_node->edges_.push_back(make_foot_edge(station_node, route_node));
  }

  if (!out_allowed) {
    route_node->edges_.push_back(make_invalid_edge(route_node, station_node));
  } else {
    route_node->edges_.push_back(
        make_foot_edge(route_node, station_node, transfer_time, true));
  }

  if (station_node->foot_node_ && out_allowed) {
    route_node->edges_.push_back(
        make_after_train_edge(route_node, station_node->foot_node_, 0, true));
  }

  return route_node;
}

}  // namespace motis
