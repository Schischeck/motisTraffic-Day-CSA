#ifndef TDGRAPH_H
#define TDGRAPH_H TDGRAPH_H

#include <vector>
#include <string>
#include <memory>

#include <pugixml.hpp>

#include "ParetoDijkstra.h"
#include "Arrival.h"
#include "Label.h"
#include "Journey.h"
#include "LowerBounds.h"

namespace td
{

struct Schedule;

class Graph
{
public:
  Graph(Schedule& schedule, MemoryManager<Label>& labelStore);

  std::vector<Journey> getConnections(
      Arrival from, Arrival to,
      int time1, int time2, int day,
      ParetoDijkstra::Statistics* stats = nullptr);

  void outputPathCompact(Journey const& journey, std::ostream& out);
  void outputPathXML(Journey const& journey, pugi::xml_node& connections);

  void generateStartLabels(
      Time const from,
      Time const to,
      StationNode const* station,
      std::vector<Label*>& indices,
      StationNode const* realStart,
      int timeOff,
      int startPrice,
      int slot,
      LowerBounds& context);

  void generateStartLabels(
      Time const from,
      Time const to,
      StationNode const* stationNode,
      Node const* route,
      std::vector<Label*>& indices,
      StationNode const* realStart,
      int timeOff,
      int startPrice,
      int slot,
      LowerBounds& context);

  Schedule& _sched;
  MemoryManager<Label>& _labelStore;
};

}

#endif //TDGRAPH_H

