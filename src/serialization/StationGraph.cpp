#include "serialization/StationGraph.h"

#include "serialization/Schedule.h"
#include "serialization/Files.h"

namespace td {

void writeStationGraph(Schedule const& sched, std::string const& prefix)
{
  std::ofstream out(prefix + STATION_GRAPH);
  out.exceptions(std::ios_base::failbit);

  for (auto& stationNode : sched.stationNodes)
  {
    auto const& fromEva = sched.stations[stationNode->_id]->evaNr;
    for (auto& stationGraphEdge : stationNode->getStationGraphEdges())
    {
      int toNodeId, clasz;
      std::tie(toNodeId, clasz) = stationGraphEdge;
      auto const& toEva = sched.stations[toNodeId]->evaNr;
      out << fromEva << ";" << toEva << ";" << clasz << ";\n";
    }
  }
}

}  // namespace td

