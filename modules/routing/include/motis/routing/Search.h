#ifndef TDGRAPH_H
#define TDGRAPH_H TDGRAPH_H

#include <vector>
#include <string>
#include <memory>

#include "motis/routing/LowerBounds.h"
#include "motis/routing/ParetoDijkstra.h"
#include "motis/routing/Arrival.h"
#include "motis/routing/Label.h"
#include "motis/routing/Journey.h"

namespace td
{

struct Schedule;

class Search
{
public:
  Search(Schedule& schedule, MemoryManager<Label>& labelStore);

  std::vector<Journey> getConnections(
      Arrival from, Arrival to,
      int time1, int time2, int day,
      ParetoDijkstra::Statistics* stats = nullptr);

  void outputPathCompact(Journey const& journey, std::ostream& out);

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

