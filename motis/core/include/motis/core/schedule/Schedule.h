#ifndef TD_SERIALIZATION_SCHEDULE_H_
#define TD_SERIALIZATION_SCHEDULE_H_

#include <map>
#include <vector>

#include "boost/filesystem.hpp"

#include "motis/core/schedule/DateManager.h"
#include "motis/core/schedule/Station.h"
#include "motis/core/schedule/Nodes.h"
#include "motis/core/schedule/ConstantGraph.h"
#include "motis/core/schedule/Attribute.h"
#include "motis/core/common/Synchronization.h"

namespace td {

class Connection;
class ConnectionInfo;

struct Schedule
{
  virtual ~Schedule() {}

  DateManager dateManager;
  std::vector<StationPtr> stations;
  std::vector<std::string> categoryNames;
  std::map<int, std::string> tracks;
  std::map<int, Attribute> attributes;
  ConstantGraph lowerBounds;
  unsigned nodeCount;
  std::vector<StationNodePtr> stationNodes;
  Synchronization sync;
};

typedef std::unique_ptr<Schedule> SchedulePtr;

struct TextSchedule : public Schedule {
  std::vector<std::unique_ptr<Connection>> fullConnections;
  std::vector<std::unique_ptr<ConnectionInfo>> connectionInfos;
};

struct BinarySchedule : public Schedule {
  std::unique_ptr<char[]> rawMemory;
};

}  // namespace td

#endif  // TD_SERIALIZATION_SCHEDULE_H_
