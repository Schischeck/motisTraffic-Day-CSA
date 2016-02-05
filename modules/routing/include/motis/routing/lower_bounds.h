#pragma once

#include "motis/core/schedule/constant_graph.h"

//#define WITH_PRICES

namespace motis {

struct lower_bounds {
  lower_bounds(
      constant_graph const& graph, int goal,
      std::unordered_map<int, std::vector<simple_edge>> const& additional_edges)
      : travel_time(graph, goal, additional_edges),
        transfers(graph, goal, additional_edges) {}

  constant_graph_dijkstra<0> travel_time;
  constant_graph_dijkstra<1> transfers;
};

}  // namespace motis
