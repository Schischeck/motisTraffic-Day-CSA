#ifndef TD_SERIALIZATION_DESERIALIZER_H_
#define TD_SERIALIZATION_DESERIALIZER_H_

#include <memory>
#include <vector>
#include <string>

#include "Array.h"
#include "Connection.h"
#include "Station.h"
#include "Nodes.h"
#include "serialization/Offset.h"

namespace td {

class Deserializer {
public:
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

  Deserializer(std::string const& prefix);

  std::pair<int, std::unique_ptr<char[]>>
  loadGraph(std::vector<StationPtr>& stations,
            std::vector<StationNodePtr>& stationNodes);

private:
  std::string _prefix;
};

}  // namespace td

#endif  // TD_SERIALIZATION_DESERIALIZER_H_
