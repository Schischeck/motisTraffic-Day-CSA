#pragma once

#include <unordered_map>
#include <vector>

#include "motis/core/schedule/constant_graph.h"

namespace motis {

struct lower_bounds {
  lower_bounds(
      constant_graph const& graph, int goal,
      std::unordered_map<int, std::vector<simple_edge>> const& additional_edges)
      : travel_time_(graph, goal, additional_edges),
        transfers_(graph, goal, additional_edges) {}

  constant_graph_dijkstra<0> travel_time_;
  constant_graph_dijkstra<1> transfers_;
};

}  // namespace motis
