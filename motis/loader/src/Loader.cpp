#include "motis/loader/Loader.h"

#include "boost/filesystem.hpp"

#include "motis/loader/BitsetManager.h"
#include "motis/loader/GraphLoader.h"
#include "motis/loader/Deserializer.h"
#include "motis/loader/Files.h"

namespace td {

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

SchedulePtr loadSchedule(std::string const& prefix)
{
  if (boost::filesystem::exists(prefix + SCHEDULE_FILE))
    return loadBinarySchedule(prefix);
  else
    return loadTextSchedule(prefix);
}

SchedulePtr loadTextSchedule(std::string const& prefix)
{
  std::unique_ptr<TextSchedule> s(new TextSchedule());

  std::map<std::string, int> classes;

  GraphLoader loader(prefix);
  loader.loadDates(s->dateManager);
  int nodeId = loader.loadStations(s->stations, s->stationNodes);
  loader.loadClasses(classes);
  loader.loadCategoryNames(s->categoryNames);
  nodeId = loader.loadRoutes(nodeId, s->stations, s->stationNodes,
                             buildCategoryClassMap(s->categoryNames, classes),
                             s->fullConnections, s->connectionInfos,
                             s->routeIndexToFirstRouteNode);
  nodeId = loader.loadFootPaths(nodeId, s->stationNodes);
  loader.loadTracks(s->tracks);
  loader.loadAttributes(s->attributes);
  loader.assignPredecessors(s->stationNodes);

  s->nodeCount = nodeId;

  s->lowerBounds = ConstantGraph(s->stationNodes);

  return SchedulePtr(s.release());
}

SchedulePtr loadBinarySchedule(std::string const& prefix)
{
  std::unique_ptr<BinarySchedule> s(new BinarySchedule());

  GraphLoader loader(prefix);
  loader.loadDates(s->dateManager);
  loader.loadCategoryNames(s->categoryNames);
  loader.loadTracks(s->tracks);
  loader.loadAttributes(s->attributes);

  Deserializer deserializer(prefix);
  std::tie(s->nodeCount, s->rawMemory) =
      deserializer.loadGraph(s->stations, s->stationNodes);

  s->lowerBounds = ConstantGraph(s->stationNodes);

  return SchedulePtr(s.release());
}

}  // namespace td
