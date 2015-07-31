#ifndef TD_INDEX_H_
#define TD_INDEX_H_

#include "motis/core/common/Offset.h"
#include "motis/core/common/Array.h"

namespace td {

struct Index {
  typedef Offset<Connection*>::type ConnectionOffset;
  typedef Offset<ConnectionInfo*>::type ConnectionInfoOffset;
  typedef Offset<StationNode*>::type StationNodeOffset;
  typedef Offset<Station*>::type StationOffset;

  Index(unsigned nodeCount,
        std::size_t stationCount,
        std::size_t fullConnectionCount,
        std::size_t connectionInfoCount,
        std::size_t stationNodeCount)
      : nodeCount(static_cast<uint32_t>(nodeCount)),
        stations(stationCount),
        fullConnections(fullConnectionCount),
        connectionInfos(connectionInfoCount),
        stationNodes(stationNodeCount)
  {}

  uint32_t nodeCount;
  Array<StationOffset> stations;
  Array<ConnectionOffset> fullConnections;
  Array<ConnectionInfoOffset> connectionInfos;
  Array<StationNodeOffset> stationNodes;
};

}  // namespace td

#endif  // TD_INDEX_H_