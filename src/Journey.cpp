#include "Journey.h"

#include "boost/lexical_cast.hpp"

#include "Label.h"
#include "TDTime.h"
#include "IntervalMap.h"
#include "serialization/Schedule.h"

#define UNKNOWN_TRACK (1)

namespace td {

namespace intermediate {

struct Stop
{
  Stop() = default;
  Stop(int index,
       int stationId,
       int aPlatform, int dPlatform,
       Time aTime, Time dTime,
       bool interchange)
      : index(index),
        stationId(stationId),
        aPlatform(aPlatform),
        dPlatform(dPlatform),
        aTime(aTime),
        dTime(dTime),
        interchange(interchange)
  {}

  int index;
  int stationId;
  int aPlatform, dPlatform;
  Time aTime, dTime;
  bool interchange;
};

struct Transport
{
  Transport() = default;
  Transport(int from, int to,
            LightConnection const* con)
      : from(from),
        to(to),
        con(con),
        duration(con->aTime - con->dTime),
        slot(-1)
  {}

  Transport(int from, int to, int duration, int slot)
      : from(from),
        to(to),
        con(nullptr),
        duration(duration),
        slot(slot)
  {}

  int from, to;
  LightConnection const* con;
  int duration;
  int slot;
};

std::pair<std::vector<intermediate::Stop>,
          std::vector<intermediate::Transport>>
  parseLabelChain(Label const* label)
{
  std::vector<Label const*> labels;
  Label const* c = label;
  do { labels.insert(begin(labels), c); } while ((c = c->_pred));

  std::vector<intermediate::Stop> stops;
  std::vector<intermediate::Transport> transports;
  enum State {
    AT_STATION,
    PRE_CONNECTION,
    IN_CONNECTION,
    WALK
  } state = AT_STATION;
  LightConnection const* lastCon = nullptr;
  Time walkArrival = INVALID_TIME;
  int stationIndex = -1;

  for (auto it = begin(labels); it != end(labels); ++it)
  {
    auto current = *it;

    if (state == IN_CONNECTION && current->_connection == nullptr)
      state = current->_node->isStationNode() ? AT_STATION : WALK;

    if (state == AT_STATION &&
        std::next(it) != end(labels) &&
        (*std::next(it))->_node->isStationNode())
    {
      state = WALK;
    }

    switch (state)
    {
      case AT_STATION:
      {
        int aPlatform = UNKNOWN_TRACK;
        int dPlatform = UNKNOWN_TRACK;
        Time aTime = walkArrival;
        Time dTime = INVALID_TIME;
        if (aTime == INVALID_TIME && lastCon != nullptr)
        {
          aPlatform = lastCon->_fullCon->aPlatform;
          aTime = lastCon->aTime;
        }

        walkArrival = INVALID_TIME;

        auto s1 = std::next(it, 1);
        auto s2 = std::next(it, 2);
        if (s1 != end(labels) && s2 != end(labels) &&
            (*s2)->_connection != nullptr)
        {
          dPlatform = (*s2)->_connection->_fullCon->dPlatform;
          dTime = (*s2)->_connection->dTime;
        }

        stops.emplace_back(
            ++stationIndex,
            current->_node->getStation()->_id,
            aPlatform, dPlatform,
            aTime, dTime,
            aTime != INVALID_TIME &&
                dTime != INVALID_TIME &&
                lastCon != nullptr);

        state = PRE_CONNECTION;
        break;
      }

      case PRE_CONNECTION:
        state = IN_CONNECTION;
        break;

      case WALK:
        assert(std::next(it) != end(labels));

        stops.emplace_back(
            ++stationIndex,
            current->_node->getStation()->_id,
            lastCon == nullptr ? UNKNOWN_TRACK : lastCon->_fullCon->aPlatform,
            UNKNOWN_TRACK,
            stops.empty() ? INVALID_TIME : current->_now,
            current->_now,
            lastCon != nullptr);

        transports.emplace_back(
            stationIndex, stationIndex + 1,
            (*std::next(it))->_now - current->_now,
            -1);

        walkArrival = (*std::next(it))->_now;

        lastCon = nullptr;

        state = AT_STATION;
        break;

      case IN_CONNECTION:
        transports.emplace_back(
            stationIndex, stationIndex + 1, current->_connection);

        // Do not collect the last connection route node.
        assert(std::next(it) != end(labels));
        auto succ = *std::next(it);
        if (succ->_node->isRouteNode())
        {
          stops.emplace_back(
              ++stationIndex,
              current->_node->getStation()->_id,
              current->_connection->_fullCon->aPlatform,
              succ->_connection->_fullCon->dPlatform,
              current->_connection->aTime,
              succ->_connection->dTime,
              false);
        }

        lastCon = current->_connection;
        break;
    }
  }

  transports.front().slot = label->getSlot(true);
  transports.back().slot = label->getSlot(false);

  return {stops, transports};
}

}  // namespace intermediate

Journey::Transport generateJourneyTransport(
    int from, int to, intermediate::Transport const& t,
    Schedule const& sched)
{
  bool walk = false;
  std::string name;
  std::string catName;
  int catId = 0;
  int trainNr = 0;
  int duration = t.duration;
  int slot = -1;

  if (t.con == nullptr)
  {
    walk = true;
    slot = t.slot;
  }
  else
  {
    ConnectionInfo const* conInfo = t.con->_fullCon->conInfo;
    catId = conInfo->family;
    catName = sched.categoryNames[catId];
    name = catName + " ";
    if (conInfo->trainNr != 0)
      name += boost::lexical_cast<std::string>(conInfo->trainNr);
    else
      name += conInfo->lineIdentifier;
  }

  return { from, to, walk, name, catName, catId, trainNr, duration, slot };
}

std::vector<Journey::Transport> generateJourneyTransports(
    std::vector<intermediate::Transport> const& transports,
    Schedule const& sched)
{
  auto conInfoEq = [](ConnectionInfo const* a,
                      ConnectionInfo const* b) -> bool
  {
    if (a == nullptr || b == nullptr)
      return false;
    else
      // Equals comparison ignoring attributes:
      return a->lineIdentifier == b->lineIdentifier &&
             a->family == b->family &&
             a->trainNr == b->trainNr &&
             a->service == b->service;
  };

  std::vector<Journey::Transport> journeyTransports;

  bool issetLast = false;
  intermediate::Transport const* last = nullptr;
  ConnectionInfo const* lastConInfo = nullptr;
  int from = 1;

  for (auto const& transport : transports)
  {
    ConnectionInfo const* conInfo = nullptr;
    if (transport.con != nullptr)
      conInfo = transport.con->_fullCon->conInfo;

    if (!conInfoEq(conInfo, lastConInfo))
    {
      if (last != nullptr && issetLast)
      {
        journeyTransports.push_back(
            generateJourneyTransport(from, transport.from, *last, sched));
      }

      issetLast = true;
      last = &transport;
      from = transport.from;
    }

    lastConInfo = conInfo;
  }

  auto back = transports.back();
  journeyTransports.push_back(
      generateJourneyTransport(from, back.to, *last, sched));

  return journeyTransports;
}

std::vector<Journey::Stop> generateJourneyStops(
    std::vector<intermediate::Stop> const& stops,
    Schedule const& sched)
{
  auto getPlatform = [](Schedule const& sched, int platformId) -> std::string
  {
    auto it = sched.tracks.find(platformId);
    return it == end(sched.tracks) ? "?" : it->second;
  };

  std::vector<Journey::Stop> journeyStops;
  for (auto const& stop : stops)
    journeyStops.push_back(
        {
          stop.index,
          stop.interchange,
          sched.stations[stop.stationId]->name,
          sched.stations[stop.stationId]->evaNr,
          sched.stations[stop.stationId]->width,
          sched.stations[stop.stationId]->length,
          stop.aTime != INVALID_TIME
            ? Journey::Stop::EventInfo
                { true,
                  sched.dateManager.formatISO(stop.aTime),
                  getPlatform(sched, stop.aPlatform) }
            : Journey::Stop::EventInfo
                { false, "", "" },
          stop.dTime != INVALID_TIME
            ? Journey::Stop::EventInfo
                { true,
                  sched.dateManager.formatISO(stop.dTime),
                  getPlatform(sched, stop.dPlatform) }
            : Journey::Stop::EventInfo
                { false, "", "" }
        });
  return journeyStops;
}

std::vector<Journey::Attribute> generateJourneyAttributes(
    std::vector<intermediate::Transport> const& transports,
    Schedule const& sched)
{
  IntervalMap attributes;
  for (auto const& transport : transports)
    if (transport.con == nullptr)
      continue;
    else
      for (auto const& attribute : transport.con->_fullCon->conInfo->attributes)
        attributes.addEntry(attribute, transport.from, transport.to);

  std::vector<Journey::Attribute> journeyAttributes;
  for (auto const& attribute : attributes.getAttributeRanges())
  {
    auto const& attributeId = attribute.first;
    auto const& attributeRanges = attribute.second;
    auto const& code = sched.attributes.at(attributeId)._code;
    auto const& text = sched.attributes.at(attributeId)._str;

    for (auto const& range : attributeRanges)
      journeyAttributes.push_back({ range.from, range.to, code, text });
  }

  return journeyAttributes;
}

std::string generateDate(Label const* label, Schedule const& sched)
{
  int startDay = label->_start / MINUTES_A_DAY;
  DateManager::Date date = sched.dateManager.getDate(startDay);
  return boost::lexical_cast<std::string>(date.day) + "." +
         boost::lexical_cast<std::string>(date.month) + "." +
         boost::lexical_cast<std::string>(date.year);
}

Journey::Journey(Label const* label, Schedule const& sched)
{
  auto parsed = intermediate::parseLabelChain(label);
  std::vector<intermediate::Stop> const& s = parsed.first;
  std::vector<intermediate::Transport> const& t = parsed.second;

  stops = generateJourneyStops(s, sched);
  transports = generateJourneyTransports(t, sched);
  attributes = generateJourneyAttributes(t, sched);

  date = generateDate(label, sched);
  duration = label->_travelTime[0];
  transfers = label->_transfers[0] - 1;
  price = label->_totalPrice[0];
}

}  // namespace td