#include "motis/prep/StationGraph.h"

#include <ostream>
#include <fstream>

#include "motis/core/schedule/Schedule.h"
#include "motis/loader/Files.h"

namespace td {

typedef std::pair<int /* to node id */, int /* train class */> StationGraphEdge;

std::vector<StationGraphEdge> stationGraphEdges(StationNode const* node)
{
  std::vector<StationGraphEdge> edges;

  for (auto const& edge : node->_edges)
  {
    // Destination can be:
    // - a route node of this station
    // - the foot node of this station
    Node const* edgeDest = edge.getDestination();

    // Destination of edges for route/foot node can be:
    // - for route nodes: back to station node (this station)
    // - for route nodes: route edge to another station
    // - for foot nodes: station node of another station
    for (auto const& edgeDestEdge : edgeDest->_edges)
    {
      // Skip foot edges and empty route edges.
      if (edgeDestEdge.empty())
        continue;

      // For a route edge:
      edges.emplace_back(
          edgeDestEdge.getDestination()->getStation()->_id,
          edgeDestEdge._m._routeEdge._conns[0]._fullCon->clasz);
    }
  }

  std::sort(std::begin(edges), std::end(edges));
  edges.erase(
      std::unique(std::begin(edges), std::end(edges)),
      std::end(edges));

  return edges;
}

void writeStationGraph(Schedule const& sched, std::string const& prefix)
{
  std::ofstream out(prefix + STATION_GRAPH);
  out.exceptions(std::ios_base::failbit);

  for (auto& stationNode : sched.stationNodes)
  {
    auto const& fromEva = sched.stations[stationNode->_id]->evaNr;
    for (auto& stationGraphEdge : stationGraphEdges(stationNode.get()))
    {
      int toNodeId, clasz;
      std::tie(toNodeId, clasz) = stationGraphEdge;
      auto const& toEva = sched.stations[toNodeId]->evaNr;
      out << fromEva << ";" << toEva << ";" << clasz << ";\n";
    }
  }
}

}  // namespace td

