#include "GraphLoader.h"

#include <cstdio>
#include <cmath>
#include <set>
#include <sstream>
#include <string>

#include "Station.h"
#include "BitsetManager.h"
#include "DateManager.h"
#include "Edges.h"
#include "TDTime.h"
#include "Nodes.h"
#include "Connection.h"
#include "serialization/Files.h"

#ifndef M_PI
#define M_PI (3.14159265359)
#endif

#define d2r (M_PI / 180.0)

using namespace std;

namespace td
{

template<typename T>
std::vector<T> join(std::vector<std::vector<T>> vecs)
{
  if (vecs.empty())
    return {};
  else if (vecs.size() == 1)
    return vecs[0];
  else
  {
    std::vector<T> ret = std::move(*std::begin(vecs));
    std::for_each(
        std::next(std::begin(vecs)),
        std::end(vecs),
        [&ret](std::vector<T> const& vec) {
          ret.insert(std::end(ret), std::begin(vec), std::end(vec));
        }
    );
    return ret;
  }
}

GraphLoader::GraphLoader(const string& prefix)
    : _prefix(prefix)
{}

std::istream& operator>>(std::istream& in, Station& station)
{
  char c;
  in >> station.index >> c;
  std::string s;
  getline(in, s);
  int i1 = s.find('|');
  int i2 = s.find('|', i1 + 1);
  station.evaNr = s.substr(0, i1);
  station.name = s.substr(i1 + 1, i2 - i1 - 1);

  int i3 = s.rfind('|');
  s = s.substr(i3 + 1, s.size() - i3 - 1);

  unsigned timeDifference;
  unsigned kMInfo;
  unsigned footpathsOut;
  unsigned footpathsIn;
  istringstream iss(s);
  iss >> timeDifference >> station.length >> station.width
      >> kMInfo >> station.usHoch >> station.usNieder
      >> footpathsOut >> footpathsIn;

  return in;
}

int GraphLoader::loadStations(
    std::vector<StationPtr>& stations,
    std::vector<StationNodePtr>& stationNodes)
{
  std::ifstream in;
  in.exceptions(ios_base::failbit);
  in.open(_prefix + STATIONS_FILE);

  int one;
  in >> one;
  assert(one == 1);
  std::string dummy;
  std::getline(in, dummy);

  int nodeId = 0;

  Station dummy1;
  dummy1.evaNr = "-1";
  dummy1.index = nodeId;
  dummy1.name = "DUMMY";

  stations.emplace_back(new Station(dummy1), Deleter<Station>(true));
  stationNodes.emplace_back(new StationNode(nodeId++), Deleter<StationNode>(true));

  int i = 1;
  while(!in.eof())
  {
    if (in.peek() == '\n' || in.eof())
    {
      assert(in.eof());
      break;
    }

    stations.emplace_back(new Station(), Deleter<Station>(true));
    in >> *stations[i];
    if (stations[i]->index != i)
      std::cout << i << " " << stations[i]->index << "\n";
    assert(stations[i]->index == i);

    stationNodes.emplace_back(new StationNode(nodeId++), Deleter<StationNode>(true));
    assert(stationNodes[i]->_id == static_cast<unsigned>(i));

    ++i;
  }

  Station dummy2 = dummy1;
  dummy2.evaNr = "-2";
  dummy2.index = nodeId;
  dummy2.name = "DUMMY";

  stations.emplace_back(new Station(dummy2), Deleter<Station>(true));
  stationNodes.emplace_back(new StationNode(nodeId++), Deleter<StationNode>(true));

  return nodeId;
}

void GraphLoader::loadBitfields(BitsetManager& bm)
{
  std::ifstream in;
  in.exceptions(ios_base::failbit);
  in.open(_prefix + BITFIELDS_FILE);
  bm = BitsetManager(in);
}

int GraphLoader::loadRoutes(
    int nodeId,
    std::vector<StationPtr>& stations,
    std::vector<StationNodePtr> const& stationNodes,
    BitsetManager& bm,
    const std::map<int, int>& classMapping,
    std::vector<std::unique_ptr<Connection>>& fullConnections,
    std::vector<std::unique_ptr<ConnectionInfo>>& connectionInfos)
{
  std::ifstream in;
  in.exceptions(ios_base::failbit);
  in.open(_prefix + ROUTES_FILE);

  uint64_t lightConId = 0;
  std::vector<LightConnection*> connections;

  uint32_t conInfoId = 0;
  std::map<ConnectionInfo, uint32_t> conInfos;

  int index;
  while(!in.eof())
  {
    if (in.peek() == '\n' || in.eof())
    {
      assert(in.eof());
      break;
    }

    char c;
    unsigned nStations, nTrains;
    in >> c;
    assert(c == 'r');
    in >> index;
    in >> nStations >> nTrains;
    assert(nStations > 1);
    assert(nTrains > 0);

    vector<bool> skipArrival(nStations), skipDeparture(nStations);
    vector<int> locations(nStations), family(nStations - 1);
    for(unsigned i = 0; i < nStations; ++i)
    {
      if(i > 0)
      {
        in >> c;
        assert(c == ',');
      }
      in >> c;
      assert(c == 'l');
      in >> locations[i];

      if(i > 0)
      {
        in >> c;
        assert(c == 'a');
        int temp;
        in >> temp;
        skipArrival[i] = (temp == 1);
      }
      if(i < nStations - 1)
      {
        in >> c;
        assert(c == 'd');
        int temp;
        in >> temp >> family[i];
        skipDeparture[i] = (temp == 1);
      }
    }

    // First layer: section between stations: (nStations - 1) sections
    // Second layer: elementary connections (not expanded bitfields)
    vector<vector<InputConnection>> inputConnections(nStations - 1);
    for(unsigned trainI = 0; trainI < nTrains; ++trainI)
    {
      in >> c;
      assert(c == 't');
      int index2, service;
      in >> index2;
      in >> c;
      assert(c == 's');
      in >> service;

      //associate the route index with this service index
      for(unsigned stationI = 0; stationI < nStations; ++stationI)
      {
        if(stationI > 0)
        {
          in >> c;
          assert(c == 'a');
          in >> inputConnections[stationI - 1][trainI].aTime
              >> inputConnections[stationI - 1][trainI].aPlatform;
          inputConnections[stationI - 1][trainI].service = service;
        }

        if(stationI < nStations - 1)
        {
          if(trainI == 0)
            inputConnections[stationI].resize(nTrains);

          Station& fromStation = *stations[locations[stationI]];
          Station& toStation = *stations[locations[stationI + 1]];

          int fam = family[stationI];
          std::map<int, int>::const_iterator it;
          if (fam != -1)
          {
            it = classMapping.find(fam);

            assert(it != classMapping.end());
            assert(it->second < 10);

            ++fromStation.depClassEvents[it->second];
            ++toStation.arrClassEvents[it->second];

            inputConnections[stationI][trainI].clasz = it->second;
          }

          inputConnections[stationI][trainI].family = fam;
          inputConnections[stationI][trainI].price =
              getDistance(fromStation, toStation) *
              Prices::getPricePerKm(inputConnections[stationI][trainI].clasz);

          in >> c;
          assert(c == 'd');
          int tdi;
          int transferClassDummy;
          int linienzuordnungDummy;
          in >> inputConnections[stationI][trainI].dTime
             >> inputConnections[stationI][trainI].dPlatform
             >> tdi
             >> transferClassDummy
             >> linienzuordnungDummy;
          inputConnections[stationI][trainI].trafficDayIndex = bm.getNewIndex(tdi);

          int n;
          in >> n;
          inputConnections[stationI][trainI].attributes.resize(n);
          for(int k = 0; k < n; ++k)
            in >> inputConnections[stationI][trainI].attributes[k];

          in >> inputConnections[stationI][trainI].trainNr;

          in >> c;
          assert(c == '%');

          std::string evaNrDummy;
          getline(in, evaNrDummy, '|');
          getline(in, inputConnections[stationI][trainI].lineIdentifier, '|');

          in >> c;
          assert(c == ',');
        }
      }
    }

    //expand connections to have absolute times
    //(remove bitfield dependency)
    // First layer: section between stations: (nStations - 1) sections
    // Second layer: elementary trains (not expanded bitfields)
    // Third layer: expanded connections (expanded, absolute times)
    vector<vector<vector<LightConnection>>> expandedConnections(nStations - 1);
    for (unsigned trainI = 0; trainI < nTrains; ++trainI)
    {
      for (unsigned conI = 0; conI < inputConnections.size(); ++conI)
      {
        if (expandedConnections[conI].size() != nTrains)
          expandedConnections[conI].resize(nTrains);

        ConnectionInfo conInfo(inputConnections[conI][trainI]);
        auto it = conInfos.find(conInfo);
        if (it == std::end(conInfos))
          std::tie(it, std::ignore) =
              conInfos.insert(std::make_pair(std::move(conInfo), conInfoId++));

        Connection* fullCon = new Connection(inputConnections[conI][trainI]);
        fullCon->conInfoId = it->second;
        fullConnections.emplace_back(fullCon);

        std::tie(lightConId, expandedConnections[conI][trainI]) =
            expandBitfields(lightConId, fullCon,
                            inputConnections[conI][trainI],
                            bm);

        std::sort(
          std::begin(expandedConnections[conI][trainI]),
          std::end(expandedConnections[conI][trainI])
        );
      }
    }

    //build connection sequences
    for (unsigned trainI = 0; trainI < nTrains; ++trainI)
    {
      for (unsigned stationI = 1; stationI < nStations - 1; ++stationI)
      {
        auto& pred = expandedConnections[stationI - 1][trainI];
        auto& succ = expandedConnections[stationI][trainI];

        for (auto& predCon : pred)
        {
          auto nextIt = std::lower_bound(
            std::begin(succ),
            std::end(succ),
            LightConnection(predCon.aTime)
          );

          if (nextIt == std::end(succ))
            continue;

          assert(predCon.aTime != INVALID_TIME);
          assert(nextIt->dTime != INVALID_TIME);

          predCon._nextId = nextIt->_conId;
        }
      }
    }

    //build graph for the route
    Node* prevRouteNode = nullptr;
    for(unsigned stationI = 0; stationI < nStations; ++stationI)
    {
      StationNode* station = stationNodes[locations[stationI]].get();

      //build the new route node
      Node* routeNode = new Node(station, nodeId++);
      routeNode->_route = index;

      if(stationI > 0)
      {
        if(!skipArrival[stationI])
        {
          routeNode->_edges.emplace_back(
              makeFootEdge(stationNodes[locations[stationI]].get(),
                           stations[locations[stationI]]->getTransferTime(),
                           true));
        }

        prevRouteNode->_edges.emplace_back(
            makeRouteEdge(routeNode, join(expandedConnections[stationI - 1])));

        for (auto& conn : prevRouteNode->_edges.back()._m._routeEdge._conns)
        {
          if (connections.size() < conn._conId + 1)
            connections.resize(conn._conId + 1);
          connections[conn._conId] = &conn;
        }
      }

      if(stationI < nStations - 1 && !skipDeparture[stationI])
        station->_edges.push_back(makeFootEdge(routeNode));
      else
        station->_edges.push_back(makeInvalidEdge(routeNode));

      prevRouteNode = routeNode;
    }

    std::string dummy;
    getline(in, dummy);
  }

  //initialize pointers to the following connection of the same train
  for (auto& connection : connections)
  {
    assert(connection != nullptr);
    if (connection->_nextId != LightConnection::INVALID_CON_ID)
      connection->_next = connections[connection->_nextId];
    else
      connection->_next = nullptr;
  }

  //reverse mapping of connection infos
  std::map<uint32_t, ConnectionInfo*> conInfoId2ConInfoPtr;
  for (auto const& conInfo : conInfos)
  {
    ConnectionInfo* info = new ConnectionInfo(conInfo.first);
    conInfoId2ConInfoPtr[conInfo.second] = info;
    connectionInfos.emplace_back(info);
  }

  //initialize pointers from the full connections to connection infos
  for (auto& fullCon : fullConnections)
    fullCon->conInfo = conInfoId2ConInfoPtr[fullCon->conInfoId];

  return nodeId;
}

std::pair<uint64_t, std::vector<LightConnection>> GraphLoader::expandBitfields(
    uint64_t lightConId,
    Connection const* fullCon,
    InputConnection const& con,
    BitsetManager const& bm)
{
  std::vector<LightConnection> ret;

  for (auto const& day : bm.getActiveDayIndices(con.trafficDayIndex))
  {
    Time dTime = (con.dTime == INVALID_TIME) ? INVALID_TIME
                                             : day * MINUTES_A_DAY + con.dTime,
         aTime = (con.aTime == INVALID_TIME) ? INVALID_TIME
                                             : day * MINUTES_A_DAY + con.aTime;

    if (dTime != INVALID_TIME &&
        aTime != INVALID_TIME &&
        dTime > aTime)
    {
      aTime += MINUTES_A_DAY;
    }

    ret.emplace_back(lightConId++, dTime, aTime, fullCon);
  }

  return { lightConId, ret };
}

int GraphLoader::loadFootPaths(
    int nodeId,
    std::vector<StationNodePtr> const& stationNodes)
{
  std::ifstream in;
  in.exceptions(ios_base::failbit);
  in.open(_prefix + FOOTPATHS_FILE);

  while(!in.eof())
  {
    if (in.peek() == '\n' || in.eof())
    {
      assert(in.eof());
      break;
    }

    int from, to, duration;
    in >> from >> to >> duration;

    nodeId = stationNodes[from]->addFootEdge(
        nodeId,
        makeFootEdge(stationNodes[to].get(), duration));

    std::string dummy;
    getline(in, dummy);
  }

  return nodeId;
}

void GraphLoader::loadAttributes(
    std::map<int, Attribute>& attributesMap)
{
  std::ifstream in;
  in.exceptions(ios_base::failbit);
  in.open(_prefix + ATTRIBUTES_FILE);

  int index;
  while(!in.eof())
  {
    if (in.peek() == '\n' || in.eof())
    {
      assert(in.eof());
      break;
    }

    char c;
    in >> index >> c;

    std::string s;
    getline(in, s);
    std::size_t p1 = s.find("|");
    assert(p1 != std::string::npos);

    std::size_t p2 = s.find("|", p1 + 1);
    assert(p2 != std::string::npos);

    attributesMap[index]._str = s.substr(0, p1);
    attributesMap[index]._code = s.substr(p1 + 1, p2 - p1 - 1);
  }
}

void GraphLoader::loadClasses(std::map<std::string, int>& map)
{
  std::ifstream in;
  in.exceptions(ios_base::failbit);
  in.open(_prefix + CLASSES_FILE);

  int nClasses;
  in >> nClasses;

  int i = 0;
  while(!in.eof() && in.peek() != EOF && ++i <= nClasses)
  {
    int index;
    in >> index;

    int nCategories;
    in >> nCategories;

    std::string cat;
    for (int j = 0; j < nCategories; ++j)
    {
      in >> cat;
      map[cat] = index;
    }
  }
}

void GraphLoader::loadCategoryNames(std::vector<std::string>& categoryNames)
{
  std::ifstream in;
  in.exceptions(ios_base::failbit);
  in.open(_prefix + CATEGORIES_FILE);

  // Category indices are 1-based.
  // Push dummy category name.
  categoryNames.push_back("DUMMY");

  int index, category;
  while (!in.eof())
  {
    if (in.peek() == '\n' || in.eof())
    {
      assert(in.eof());
      break;
    }

    in >> index >> category;

    std::string s;
    getline(in, s);

    std::size_t p1 = s.find("|");
    assert(p1 != std::string::npos);

    std::size_t p2 = s.find("|", p1 + 1);
    assert(p2 != std::string::npos);

    categoryNames.push_back(s.substr(p1 + 1, p2 - p1 - 1));
  }
}

void GraphLoader::loadTracks(std::map<int, std::string>& trackMap)
{
  std::ifstream in;
  in.exceptions(ios_base::failbit);
  in.open(_prefix + TRACK_FILE);

  int nTracks;
  in >> nTracks;

  std::string buf;
  getline(in, buf);

  int index;
  while (!in.eof())
  {
    if (in.peek() == '\n' || in.eof())
    {
      assert(in.eof());
      break;
    }

    char c;
    in >> index >> c;
    assert(c == '.');

    std::string track = trackMap[index];
    getline(in, track);

    std::size_t crPos = track.find("\r");
    track = (crPos == std::string::npos) ? track : track.substr(0, crPos);
    trackMap[index] = track;
  }
}

void GraphLoader::loadDates(DateManager& dm)
{
  std::ifstream in;
  in.exceptions(ios_base::failbit);
  in.open(_prefix + DATES_FILE);

  int n;
  in >> n;
  vector<DateManager::Date> dates(n);
  char c;
  int ind;
  for(int i = 0; i < n; ++i)
    in  >> ind >> c >> dates[i].day >> c >> dates[i].month >> c
         >> dates[i].year >> c;

  dm.load(dates);
}

double GraphLoader::getDistance(const Station& s1, const Station& s2)
{
  double lat1 = s1.width, long1 = s1.length;
  double lat2 = s2.width, long2 = s2.length;
  double dlong = (long2 - long1) * d2r;
  double dlat = (lat2 - lat1) * d2r;
  double a = pow(sin(dlat/2.0), 2)
             + cos(lat1*d2r) * cos(lat2*d2r) * pow(sin(dlong/2.0), 2);
  double c = 2 * atan2(sqrt(a), sqrt(1-a));
  double d = 6367 * c;
  return d;
}

}

