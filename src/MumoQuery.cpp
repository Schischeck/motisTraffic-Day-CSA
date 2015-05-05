#include "MumoQuery.h"

#include <tuple>
#include <cassert>

#include "Query.h"
#include "Station.h"
#include "Measure.h"
#include "ParetoDijkstra.h"
#include "serialization/Schedule.h"

using namespace td;
using namespace pugi;

MumoQuery::MumoQuery(Graph& g)
  : _graph(g),
    _day_index(0),
    _time1(0), _time2(0),
    _day(0), _month(0), _year(0),
    _h1(0), _m1(0),
    _h2(0), _m2(0),
    _initialized(false)
{}

void MumoQuery::init(std::string dateTime, int interval,
                     Arrival dep, Arrival arr)
{
  Query::time begin = Query::read_time(dateTime);
  _day = std::get<Query::DAY>(begin);
  _month = std::get<Query::MONTH>(begin);
  _year = std::get<Query::YEAR>(begin);
  _h1 = std::get<Query::HOUR>(begin);
  _m1 = std::get<Query::MINUTE>(begin);
  _h2 = (_h1 + interval / 60) % 24;
  _m2 = _m1 + interval % 60;

  _time1 = _h1 * 60 + _m1;
  _time2 = _time1 + interval;

  DateManager::Date date(_day, _month, _year);
  _day_index = _graph._sched.dateManager.getDayIndex(date);
  if(_day_index == DateManager::NO_INDEX)
  {
    throw parse_exception("Date not available");
  }

  _dep = std::move(dep);
  _arr = std::move(arr);

  _initialized = true;
}

std::string MumoQuery::str() const {
  std::stringstream str;

  for (const auto& s : _dep)
  {
    str << "Source " << _graph._sched.stations[s.station]->name << ":"
        << " " << s.timeCost << "min "
        << " (" << s.price << "cent)"
        << " slot=" << static_cast<int>(s.slot)
        << "\n";
  }

  for (const auto& s : _arr)
  {
    str << "Destination " << _graph._sched.stations[s.station]->name << ":"
        << " " << s.timeCost << "min "
        << " (" << s.price << "cent)"
        << " slot=" << static_cast<int>(s.slot)
        << "\n";
  }

  str << _year << "-"
      << std::setw(2) << std::setfill('0') << _month << "-"
      << std::setw(2) << std::setfill('0') << _day << " ["
      << std::setw(2) << std::setfill('0') << _h1 << ":"
      << std::setw(2) << std::setfill('0') << _m1
      << ", "
      << std::setw(2) << std::setfill('0') << _h2 << ":"
      << std::setw(2) << std::setfill('0') << _m2
      << "]";

  return str.str();
}

bool MumoQuery::execute(xml_node& response)
{
  assert(_initialized);

  std::cout << "\n" << str() << "\n";

  // Search connections.
  std::vector<Journey> journeys;
  ParetoDijkstra::Statistics stats;
  unsigned int calculationTime = 0;
  if (!_dep.empty() && !_arr.empty()) {
    auto millis = measure<>::execution([this, &journeys, &stats]() {
      journeys = _graph.getConnections(_dep, _arr,
                                  _time1, _time2, _day_index, &stats);
    });
    calculationTime = static_cast<unsigned int>(millis);
  }
  stats.totalCalculationTime = calculationTime;
  bool error = journeys.empty();

  // Create response document.
  xml_node data = response.append_child("DataExchange");

  // Result header.
  xml_node header = data.append_child("ResultHeader");
  header.append_attribute("max-label-quit") = stats.maxLabelQuit;
  header.append_attribute("calculationTime") = calculationTime;
  header.append_attribute("servername") = "TD";

  // Write Pareto Dijkstra stats.
  std::cout << stats << "\n";
  xml_node statistics = data.append_child("Statistics");
  xml_node pdStats = statistics.append_child("ParetoDijkstra");
  pdStats.append_attribute("travel-time-lb") = stats.travelTimeLB;
  pdStats.append_attribute("transfers-lb") = stats.transfersLB;
  pdStats.append_attribute("price-lb") = stats.priceLB;
  pdStats.append_attribute("labels-created") = stats.labelsCreated;
  pdStats.append_attribute("labels-popped") = stats.labelsPopped;
  pdStats.append_attribute("pq-max-size") = stats.priorityQueueMaxSize;
  pdStats.append_attribute("labels-popped-until-first-result") =
      stats.labelsPoppedUntilFirstResult;
  pdStats.append_attribute("labels-popped-after-last-result") =
      stats.labelsPoppedAfterLastResult;
  pdStats.append_attribute("labels-dominated-by-former-labels") =
      stats.labelsDominatedByFormerLabels;
  pdStats.append_attribute("labels-dominated-by-later-labels") =
      stats.labelsDominatedByLaterLabels;
  pdStats.append_attribute("labels-dominated-by-results") =
      stats.labelsDominatedByResults;
  pdStats.append_attribute("start-label-count") = stats.startLabelCount;
  pdStats.append_attribute("labels-equals-popped") = stats.labelsEqualsPopped;

  // Let the graph generate the connection list.
  xml_node connections = data.append_child("ConnectionList");
  for (auto const& journey : journeys)
    _graph.outputPathXML(journey, connections);
  std::cout << "\n";

  return error;
}
