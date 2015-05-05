#include "Query.h"

#include <sstream>
#include <list>
#include <cassert>

#include "boost/regex.hpp"
#include "boost/lexical_cast.hpp"

#include "pugixml.hpp"

#include "serialization/Schedule.h"
#include "Measure.h"
#include "Station.h"

using namespace td;
using namespace pugi;

Query::Query(Graph& g, const StationGuesser& stationGuesser)
  : _graph(g),
    _stationGuesser(stationGuesser),
    _from(0),
    _to(0),
    _day_index(0),
    _time1(0),
    _time2(0),
    _initialized(false)
{}

void Query::initFromXML(const pugi::xml_document& query_doc,
                        bool oneHourInterval)
{
  // Check query.
  xml_node query = query_doc.child("Query");
  if (!query)
    throw parse_exception("Query node not found");

  // Get interval.
  xml_node i = query.child("Interval");
  if (!i)
    throw parse_exception("Interval node not found");

  // Read interval.
  time begin = read_time(i.child("Begin").attribute("dateTime").as_string());
  time end = read_time(i.child("End").attribute("dateTime").as_string());

  // Set interval.
  int day = std::get<DAY>(begin);
  int month = std::get<MONTH>(begin);
  int year = std::get<YEAR>(begin);
  int h1 = std::get<HOUR>(begin);
  int m1 = std::get<MINUTE>(begin);
  _time1 = h1 * 60 + m1;

  int h2 = 0, m2 = 0;
  if (oneHourInterval) {
    _time2 = (_time1 + 60) % 1440;
    h2 = _time2 / 60;
    m2 = _time2 % 60;
  } else {
    h2 = std::get<HOUR>(end);
    m2 = std::get<MINUTE>(end);
    _time2 = h2 * 60 + m2;
  }

  // Convert date to day index and check whether the day index exists.
  DateManager::Date date(day, month, year);
  _day_index = _graph._sched.dateManager.getDayIndex(date);
  if(_day_index == DateManager::NO_INDEX)
  {
    std::string dateStr = date.str();
    std::string first = _graph._sched.dateManager.firstDate().str();
    std::string last = _graph._sched.dateManager.lastDate().str();
    std::string message = "Date not available: ";
    message += dateStr + " not in ["  + first + ", " + last + "]";
    throw parse_exception(message);
  }

  // Read path description.
  xml_node path = query.child("PathDescription");
  if (!path)
    throw parse_exception("PathDescription node not found");

  // Read stations.
  const Station* depStation = readStation(path.first_child().next_sibling());
  const Station* arrStation = readStation(path.last_child());
  _from = depStation->index;
  _to = arrStation->index;

  std::stringstream str;
  str << depStation->name << " -> " << arrStation->name << ", "
      << year << "-"
      << std::setw(2) << std::setfill('0') << month << "-"
      << std::setw(2) << std::setfill('0') << day << " ["
      << std::setw(2) << std::setfill('0') << h1 << ":"
      << std::setw(2) << std::setfill('0') << m1
      << ", "
      << std::setw(2) << std::setfill('0') << h2 << ":"
      << std::setw(2) << std::setfill('0') << m2
      << "]";
  _stringRepresentation = str.str();

  _initialized = true;
}

bool Query::execute(pugi::xml_document& response)
{
  assert(_initialized);

  // Search connections.
  std::vector<Journey> journeys;

  ArrivalPart start;
  start.station = _from;
  start.timeCost = 0;
  start.price = 0;
  start.slot = 0;

  ArrivalPart target;
  target.station = _to;
  start.timeCost = 0;
  start.price = 0;
  start.slot = 0;

  unsigned int calculationTime = 0;
  ParetoDijkstra::Statistics stats;
  auto millis = measure<>::execution([this, &start, &target,
                                      &journeys, &stats]() {
    journeys = _graph.getConnections({ start }, { target },
                                     _time1, _time2, _day_index, &stats);
  });
  calculationTime = static_cast<unsigned int>(millis);
  stats.totalCalculationTime = calculationTime;

  // Create response document.
  xml_node data = response.append_child("DataExchange");

  // Result header.
  xml_node header = data.append_child("ResultHeader");
  header.append_attribute("max-label-quit") = stats.maxLabelQuit;
  header.append_attribute("calculationTime") = calculationTime;
  header.append_attribute("servername") = "TD";

  // Write Pareto Dijkstra stats.
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

  // Message.
  xml_node message = data.append_child("Message");
  xml_node msg_de = message.append_child("Text");
  msg_de.append_attribute("language") = "German";
  msg_de.append_child(node_pcdata).set_value("\"Verbindung(en) gefunden\"");
  xml_node msg_en = message.append_child("Text");
  msg_en.append_attribute("language") = "German";
  msg_en.append_child(node_pcdata).set_value("\"Connection(s) found\"");

  // Let the graph generate the connection list.
  xml_node connections = data.append_child("ConnectionList");
  for (auto const& journey : journeys)
    _graph.outputPathXML(journey, connections);
  std::cout << "\n";
  std::cout << stats << std::endl;

  return journeys.empty();
}

Query::time Query::read_time(const std::string& time_str) {
  boost::regex expr("(\\d{4})-(\\d{2})-(\\d{2})T(\\d{2}):(\\d{2})");
  boost::smatch what;
  if (boost::regex_search(time_str, what, expr)) {
    return time(boost::lexical_cast<int>(what[1]),
                boost::lexical_cast<int>(what[2]),
                boost::lexical_cast<int>(what[3]),
                boost::lexical_cast<int>(what[4]),
                boost::lexical_cast<int>(what[5]));
  } else {
    throw parse_exception(std::string("Invalid dateTime \"") + time_str + "\"");
  }
}

Station const* Query::readStation(const pugi::xml_node& station)
{
  // Check tag.
  if (std::string(station.name()) != "Station")
    throw parse_exception("Station node not found.");

  // Try to parse and translate the eva number.
  xml_attribute eva = station.attribute("EvaNo");
  eva = (eva == nullptr) ? station.attribute("evano") : eva;
  if (eva != nullptr)
  {
    int evaNumber = eva.as_int(-1);
    if (evaNumber < 0)
      throw parse_exception("Station eva could not be parsed");

    auto station = _stationGuesser.getStationByEva(evaNumber);
    if (station == nullptr)
      throw parse_exception("Station not found");

    return station;
  }

  // Try to parse the station name.
  xml_attribute name = station.attribute("name");
  if (name == nullptr)
    throw parse_exception("Invalid station");

  const std::string s = name.as_string();
  std::vector<const Station*> guess = _stationGuesser.guess(s, 1u);

  if (guess.empty())
    throw parse_exception("Could not guess station name " + s);

  return guess.at(0);
}
