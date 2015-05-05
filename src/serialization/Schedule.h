#ifndef TD_SERIALIZATION_SCHEDULE_H_
#define TD_SERIALIZATION_SCHEDULE_H_

#include <map>
#include <vector>

#include "boost/filesystem.hpp"

#include "BitsetManager.h"
#include "DateManager.h"
#include "Station.h"
#include "Nodes.h"
#include "ConstantGraph.h"
#include "GraphLoader.h"
#include "serialization/Deserializer.h"
#include "serialization/Files.h"

namespace td {

class Connection;
class ConnectionInfo;

struct Schedule
{
  Schedule() = default;

  Schedule(std::string const& prefix)
  {
    if (boost::filesystem::exists(prefix + SCHEDULE_FILE))
      loadBinary(prefix);
    else
      loadASCII(prefix);
  }

  void loadASCII(std::string const& prefix)
  {
    GraphLoader loader(prefix);
    loader.loadDates(dateManager);
    int nodeId = loader.loadStations(stations, stationNodes);
    loader.loadBitfields(bitsetManager);
    loader.loadClasses(classes);
    loader.loadCategoryNames(categoryNames);
    nodeId = loader.loadRoutes(nodeId, stations, stationNodes, bitsetManager,
                               buildCategoryClassMap(categoryNames, classes),
                               fullConnections, connectionInfos);
    nodeId = loader.loadFootPaths(nodeId, stationNodes);
    loader.loadTracks(tracks);
    loader.loadAttributes(attributes);

    nodeCount = nodeId;

    lowerBounds = ConstantGraph(stationNodes);
  }

  void loadBinary(std::string const& prefix)
  {
    GraphLoader loader(prefix);
    loader.loadDates(dateManager);
    loader.loadCategoryNames(categoryNames);
    loader.loadTracks(tracks);
    loader.loadAttributes(attributes);

    Deserializer deserializer(prefix);
    std::tie(nodeCount, rawMemory) =
        deserializer.loadGraph(stations, stationNodes);

    lowerBounds = ConstantGraph(stationNodes);
  }

  std::map<int, int> buildCategoryClassMap(
      std::vector<std::string> const& categoryNames,
      std::map<std::string, int> classes)
  {
    std::map<int, int> categoryClassMap;

    for (unsigned i = 0; i < categoryNames.size(); ++i)
    {
      auto it = classes.find(categoryNames[i]);
      if (it == end(classes))
        categoryClassMap[i] = 9;
      else
        categoryClassMap[i] = it->second;
    }

    return categoryClassMap;
  }

  DateManager dateManager;
  std::vector<StationPtr> stations;
  std::vector<std::string> categoryNames;
  std::map<int, std::string> tracks;
  std::map<int, Attribute> attributes;
  ConstantGraph lowerBounds;
  unsigned nodeCount;
  std::vector<StationNodePtr> stationNodes;

  std::unique_ptr<char[]> rawMemory;

  BitsetManager bitsetManager;
  std::vector<std::unique_ptr<Connection>> fullConnections;
  std::vector<std::unique_ptr<ConnectionInfo>> connectionInfos;
  std::map<std::string, int> classes;
};

}  // namespace td

#endif  // TD_SERIALIZATION_SCHEDULE_H_
