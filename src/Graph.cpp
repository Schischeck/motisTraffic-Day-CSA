#include "Graph.h"

#include <iostream>
#include <cassert>
#include <string>
#include <ctime>
#include <algorithm>
#include <list>
#include <set>
#include <stdexcept>
#include <iterator>
#include <unordered_map>

#include "boost/lexical_cast.hpp"

#include "pugixml.hpp"

#include "serialization/Schedule.h"
#include "Station.h"
#include "Edges.h"
#include "ParetoDijkstra.h"
#include "Label.h"
#include "LowerBounds.h"
#include "Timing.h"
#include "Logging.h"

namespace td {

void removeIntersection(Arrival& from, Arrival& to) {
  for (auto const& toArrPart : to)
    from.erase(std::remove_if(std::begin(from), std::end(from),
                              [&toArrPart](ArrivalPart const& fromArrPart) {
                                return toArrPart.station == fromArrPart.station;
                              }),
               std::end(from));
}

Graph::Graph(Schedule& schedule, MemoryManager<Label>& labelStore)
    : _sched(schedule),
      _labelStore(labelStore)
{}

std::vector<Journey> Graph::getConnections(
    Arrival from, Arrival to,
    int time1, int time2, int day,
    ParetoDijkstra::Statistics* stats)
{
  _labelStore.reset();
  removeIntersection(from, to);

  // use dummy station as virtual station representing
  // all source stations as well as all target stations
  assert(!_sched.stations.empty());
  int dummySource = _sched.stations.front()->index;
  int dummyTarget = _sched.stations.back()->index;

  // Generate additional edges for the lower bound graph.
  std::unordered_map<int, std::vector<SimpleEdge>> lbGraphEdges;
  for (auto const& arr : to)
    lbGraphEdges[dummyTarget].push_back(SimpleEdge(arr.station, arr.timeCost));
  for (auto const& arr : from)
    lbGraphEdges[arr.station].push_back(SimpleEdge(dummySource, arr.timeCost));

  // initialize lower bound graphs and
  // check if there is a path from source to target

  LowerBounds lowerBounds(_sched.lowerBounds, dummyTarget, lbGraphEdges);

  TD_START_TIMING(lowerBoundsTiming);

  TD_START_TIMING(travelLBTimeTiming);
  lowerBounds.travelTime.run();
  TD_STOP_TIMING(travelLBTimeTiming);

  TD_START_TIMING(transfersLBTiming);
  lowerBounds.transfers.run();
  TD_STOP_TIMING(transfersLBTiming);

  TD_START_TIMING(priceLBTiming);
  lowerBounds.price.run();
  TD_STOP_TIMING(priceLBTiming);

  TD_STOP_TIMING(lowerBoundsTiming);

  if(lowerBounds.travelTime.getDistance(dummySource) ==
     std::numeric_limits<uint32_t>::max())
  {
    LOG(logging::error) << "no path from source[" << dummySource << "] "
                        << "to target[" << dummyTarget << "] found";
    return {};
  }

  int day2 = day;
  if(time1 > time2)
    ++day2;

  std::vector<Label*> startLabels;
  StationNode* dummySourceStation = _sched.stationNodes[dummySource].get();
  for (const auto& s : from)
  {
    StationNode const* station = _sched.stationNodes[s.station].get();

    // generate labels at all route nodes
    // for all trains departing in the specified interval
    generateStartLabels(toTime(day, time1), toTime(day2, time2),
                        station, startLabels, dummySourceStation,
                        s.timeCost, s.price, s.slot,
                        lowerBounds);
  }

  std::unordered_map<Node const*, std::vector<Edge>> additionalEdges;
  StationNode* target = _sched.stationNodes[dummyTarget].get();
  for (auto const& arr : to)
  {
    StationNode* arrivalStation = _sched.stationNodes[arr.station].get();

    auto it = additionalEdges.find(arrivalStation);
    if (it == end(additionalEdges))
      std::tie(it, std::ignore) =
          additionalEdges.emplace(arrivalStation, std::vector<Edge>());

    it->second.emplace_back(makeMumoEdge(target,
                                         arr.timeCost, arr.price, arr.slot));
  }

  ParetoDijkstra pd(_sched.nodeCount,
                    _sched.stationNodes[dummyTarget].get(),
                    startLabels,
                    additionalEdges,
                    lowerBounds,
                    _labelStore);
  std::vector<Label*>& results = pd.search();

  if (stats != nullptr)
  {
    *stats = pd.getStatistics();
    stats->travelTimeLB = TD_TIMING_MS(travelLBTimeTiming);
    stats->transfersLB = TD_TIMING_MS(transfersLBTiming);
    stats->priceLB = TD_TIMING_MS(priceLBTiming);
  }

  std::vector<Journey> journeys;
  journeys.resize(results.size());
  std::transform(
      std::begin(results), std::end(results),
      std::begin(journeys),
      [this](Label* label) { return Journey(label, _sched); });

  return journeys;
}

void Graph::generateStartLabels(
    Time const from,
    Time const to,
    StationNode const* station,
    std::vector<Label*>& indices,
    StationNode const* realStart,
    int timeOff,
    int startPrice,
    int slot,
    LowerBounds& lowerBounds)
{
  for(auto const& edge : station->_edges)
    generateStartLabels(
        from, to,
        station, edge.getDestination(),
        indices,
        realStart, timeOff, startPrice, slot,
        lowerBounds);
}

void Graph::generateStartLabels(
    Time const from,
    Time const to,
    StationNode const* stationNode,
    Node const* routeNode,
    std::vector<Label*>& indices,
    StationNode const* realStart,
    int timeOff,
    int startPrice,
    int slot,
    LowerBounds& lowerBounds)
{
  for(auto const& edge : routeNode->_edges)
  {
    // not a route-edge?
    if(edge.getDestination() == stationNode)
      continue;

    Time t = from + timeOff;

    // Don't set label on foot node
    // this isn't neccesary in a intermodal scenario.
    if (edge._m._type != Edge::Type::ROUTE_EDGE) {
      continue;
    }

    while(t <= to + timeOff)
    {
      LightConnection const* con = edge.getConnection(t, nullptr);
      if(con == nullptr)
        break;

      t = con->dTime;

      if(t > to + timeOff)
        break;

      // was there an earlier start station?
      Label* earlier = nullptr;
      if(realStart != nullptr)
        earlier = new (_labelStore.create()) Label(realStart, nullptr, t - timeOff, lowerBounds);

      Label* stationNodeLabel = new (_labelStore.create()) Label(stationNode, earlier, t, lowerBounds);
      stationNodeLabel->_prices[Label::ADDITIONAL_PRICE] = startPrice;
      stationNodeLabel->_totalPrice[0] = startPrice;

      // create the label we are really interested in
      Label* routeNodeLabel = new (_labelStore.create()) Label(routeNode, stationNodeLabel, t, lowerBounds);
      routeNodeLabel->setSlot(true, slot);

      indices.push_back(routeNodeLabel);

      t = t + 1;
    }
  }
}

void Graph::outputPathCompact(Journey const& journey, std::ostream& out)
{
  for (auto const& transport : journey.transports)
    out << transport.from << " - " << transport.to << ": "
        << (transport.walk ? "walk" : transport.name.c_str()) << "\n";
  out << "\n";

  for (auto const& attribute : journey.attributes)
    out << attribute.from << " - " << attribute.to << ": "
        << attribute.text << "\n";
  out << "\n";

  for (auto const& stop : journey.stops)
    out << (stop.interchange ? "* " : "  ")
        << stop.index << ": " << stop.name << "\t"
        << stop.arrival.platform << " -> "
        << stop.departure.platform << " "
        << "\n";
  out << "\n";
}

void Graph::outputPathXML(Journey const& journey,
                          pugi::xml_node& connections)
{
  int id = std::distance(std::begin(connections), std::end(connections));
  auto con = connections.append_child("Connection");
  con.append_attribute("connectionId") = id;
  con.append_attribute("Date") = journey.date.c_str();
  con.append_attribute("Transfers") = journey.transfers;
  con.append_attribute("Duration") = journey.duration;
  auto price = con.append_child("Price");
  price.append_attribute("PAErrorCode") = "-1";
  price.append_attribute("estimate") = journey.price;

  auto stopList = con.append_child("StopList");
  for (auto const& stop : journey.stops)
  {
    auto xmlStop = stopList.append_child("Stop");
    xmlStop.append_attribute("evaNo") = stop.evaNo.c_str();
    xmlStop.append_attribute("name") = stop.name.c_str();
    xmlStop.append_attribute("lat") = stop.lat;
    xmlStop.append_attribute("lng") = stop.lng;
    xmlStop.append_attribute("index") = stop.index;

    if (stop.arrival.valid)
    {
      auto arrival = xmlStop.append_child("Arrival");
      arrival.append_attribute("dateTime") = stop.arrival.dateTime.c_str();
      arrival.append_attribute("platform") = stop.arrival.platform.c_str();
    }

    if (stop.departure.valid)
    {
      auto departure = xmlStop.append_child("Departure");
      departure.append_attribute("dateTime") = stop.departure.dateTime.c_str();
      departure.append_attribute("platform") = stop.departure.platform.c_str();
    }

    if (stop.interchange)
      xmlStop.append_child("InterchangeInfo");
  }

  auto journeyInfo = con.append_child("JourneyInfo");
  for (auto const& transport : journey.transports)
  {
    if (transport.walk)
    {
      auto walk = journeyInfo.append_child("Walk");
      walk.append_attribute("duration") = transport.duration;
      walk.append_attribute("from") = transport.from;
      walk.append_attribute("to") = transport.to;

      if (transport.slot != -1)
        walk.append_attribute("slot") = transport.slot;
    }
    else
    {
      auto train = journeyInfo.append_child("Transport");
      train.append_attribute("name") = transport.name.c_str();
      train.append_attribute("categoryID") = transport.categoryId;
      train.append_attribute("categoryName") = transport.categoryName.c_str();
      train.append_attribute("number") = transport.trainNr;
      train.append_attribute("from  ") = transport.from;
      train.append_attribute("to") = transport.to;
    }
  }

  for (auto const& attribute : journey.attributes)
  {
    auto xmlAttr = journeyInfo.append_child("Attribute");
    xmlAttr.append_attribute("code") = attribute.code.c_str();
    xmlAttr.append_attribute("text") = attribute.text.c_str();
    xmlAttr.append_attribute("from") = attribute.from;
    xmlAttr.append_attribute("to") = attribute.to;
  }
}

}  // namespace td