#ifndef TD_LOWER_BOUNDS_H_
#define TD_LOWER_BOUNDS_H_

#include "motis/core/schedule/ConstantGraph.h"

namespace td
{

struct LowerBounds
{
  LowerBounds(
      ConstantGraph const& graph,
      int goal,
      std::unordered_map<int, std::vector<SimpleEdge>> const& additionalEdges)
    : travelTime(graph, goal, additionalEdges),
      transfers(graph, goal, additionalEdges),
      price(graph, goal, additionalEdges)
  {}

  ConstantGraphDijkstra<0> travelTime;
  ConstantGraphDijkstra<1> transfers;
  ConstantGraphDijkstra<2> price;
};

}  // namespace td

#endif  // TD_LOWER_BOUNDS_H_