#ifndef TD_GRAPH_LOADER_H_
#define TD_GRAPH_LOADER_H_

#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include <map>
#include <set>
#include <map>

#include "Attribute.h"
#include "Nodes.h"
#include "Station.h"

namespace td {

class BitsetManager;
class DateManager;
class Node;
class Edge;
class StationNode;
class Connection;
class InputConnection;
class LightConnection;
class ConnectionInfo;

class GraphLoader
{
public:
  GraphLoader(const std::string& prefix);

  void loadDates(DateManager& dm);
  int loadStations(std::vector<StationPtr>& stations,
                   std::vector<StationNodePtr>& stationNodes);
  void loadBitfields(BitsetManager& bm);
  int loadRoutes(
      int nodeId,
      std::vector<StationPtr>& stations,
      std::vector<StationNodePtr> const& stationNodes,
      BitsetManager& bm,
      const std::map<int, int>& classMapping,
      std::vector<std::unique_ptr<Connection>>& fullConnections,
      std::vector<std::unique_ptr<ConnectionInfo>>& connectionInfos);
  int loadFootPaths(int nodeId,
                    std::vector<StationNodePtr> const& stationNodes);
  void loadAttributes(std::map<int, Attribute>& attributesMap);
  void loadClasses(std::map<std::string, int>& map);
  void loadCategoryNames(std::vector<std::string>& categoryNames);
  void loadTracks(std::map<int, std::string>& trackMap);

private:
  /** Bitfield rollout (= absolute times, relative to day 0 of schedule) */
  static std::pair<uint64_t, std::vector<LightConnection>> expandBitfields(
      uint64_t lightConId,
      Connection const* fullCon,
      InputConnection const& con,
      BitsetManager const& bm);

  /** @return the distance between the given stations. */
  double getDistance(const Station& s1, const Station& s2);

  //Prefix of schedule files
  std::string _prefix;
};

}  // namespace td

#endif  // TD_GRAPH_LOADER_H_

