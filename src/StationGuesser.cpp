#include "StationGuesser.h"

#include <cmath>
#include <iostream>
#include <fstream>
#include <algorithm>

#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>

#include "Station.h"

using namespace td;

StationGuesser::StationGuesser(std::vector<StationPtr> const& stations)
  : _stations(stations)
{
  // Preprocess station names (build 3grams, normalize).
  _stationTrigrams.resize(_stations.size());
  _normalizedStationNames.resize(_stations.size());
  _stationSizes.resize(_stations.size());
  int i = 0;
  for (const auto& s : _stations)
  {
    std::string name = s->name;

    // Fill mappings:
    // EvaNr -> Station
    // Name  -> Station
    int evaNr;
    try {
      evaNr = boost::lexical_cast<int>(s->evaNr.toString());
    } catch (boost::bad_lexical_cast&) {
      std::cout << "unable to read " << s->evaNr << "\n";
    }
    _evaStationMap[evaNr] = s.get();
    _nameStationMap[s->name] = s.get();

    // Store normalized name.
    normalize(name);
    _normalizedStationNames[i] = name;

    // Precompute trigrams.
    _stationTrigrams[i] = trigrams(name);

    // Precompute station size factor.
    _stationSizes[i] = 0;
    for (int j = 0; j < 10; ++j) {
      _stationSizes[i] += std::pow(10, (9 - j) / 3) * s->depClassEvents[j];
    }
    _stationSizes[i] = _stationSizes[i] / (double) 100;
    if (name.find("hbf") != std::string::npos) {
      _stationSizes[i] *= 1.33;
    }

    ++i;
  }
}

void StationGuesser::writeStatistics(std::string const& filename) const {
  std::ofstream out(filename.c_str());
  out.exceptions(std::ios_base::failbit);

  struct wrapper {
    wrapper(double heuristic, Station const* station)
        : heuristic(heuristic),
          station(station) {
    }

    double heuristic;
    Station const* station;

    bool operator<(wrapper const& b) const {
      return heuristic > b.heuristic;
    }

    bool operator==(wrapper const& b) const {
      return station == b.station;
    }
  };

  std::vector<wrapper> wrappers;
  for (unsigned i = 0; i < _stations.size(); ++i) {
    wrappers.emplace_back(_stationSizes[i], _stations[i].get());
  }

  std::sort(begin(wrappers), end(wrappers));

  for (auto const& w : wrappers) {
    out << w.station->index << " " << w.station->evaNr << "%"
        << w.station->name << "%";
    for (auto const& evCount : w.station->depClassEvents) {
      out << evCount << " ";
    }
    out << w.heuristic << "\n";
  }

  out << std::flush;
}

Station const* StationGuesser::getStationByName(std::string const& name) const {
  auto it = _nameStationMap.find(name);
  if (it == std::end(_nameStationMap)) {
    return nullptr;
  }
  return it->second;
}

Station const* StationGuesser::getStationByEva(std::string const& evaNr) const {
  return getStationByEva(boost::lexical_cast<int>(evaNr));
}

Station const* StationGuesser::getStationByEva(int evaNr) const {
  auto it = _evaStationMap.find(evaNr);
  if (it == std::end(_evaStationMap)) {
    return nullptr;
  }
  return it->second;
}

std::vector<const Station*> StationGuesser::guess(const std::string& input,
                                                  const std::size_t count) const
{
  // No guesses for "".
  if (input.empty())
    return std::vector<const Station*>();

  // Try to match the name exactly.
  Station const* stationByName = getStationByName(input);
  if (stationByName != nullptr)
    return { stationByName };

  // Normalize input string.
  std::string in = input;
  normalize(in);
  std::vector<std::string> inTrigrams = trigrams(in);

  // Match stations using a regular expression
  // and calculate a similarity (3gram overlap percentage) heuristic.
  // We only store matches with overlap > 30%
  const boost::regex rx(build_regex(in));
  int i = 0;
  std::vector<std::pair<int    /* station index */,
                        double /* similarity */>> candidates;
  for (const std::string& s : _normalizedStationNames)
  {
    double sim = 0.0f;
    if (boost::regex_match(s.cbegin(), s.cend(), rx) &&
        (sim = similarity(i, inTrigrams)) >= 0.3)
      candidates.push_back(std::make_pair(i, sim));
    ++i;
  }

  // Sort matches according to the similarity measure.
  std::sort(candidates.begin(), candidates.end(),
            [](std::pair<int, double> s1, std::pair<int, double> s2)
            { return s1.second > s2.second; });

  // Sort the remaining with weighted station usage (event count).
  std::sort(candidates.begin(), candidates.end(),
    [this](std::pair<int, double> s1, std::pair<int, double> s2)
    {
      return stationHeuristic(s1.second, s1.first)
           > stationHeuristic(s2.second, s2.first);
    });

  // Keep only the X best.
  candidates.resize(std::min(count, candidates.size()));

  // Create result vector with stations in it.
  std::vector<const Station*> stations(candidates.size());
  int j = 0;
  for (const auto& s : candidates) {
    stations[j++] = _stations[s.first].get();
#ifdef DEBUG_STATION_GUESSER
    std::cout << in << "\t" << std::setw(70) << stations[j - 1]->name << ": "
              << "similarity=" << s.second << ", "
              << "size=" << _stationSizes[s.first] << ", "
              << "heuristic=" << stationHeuristic(s.second, s.first) << "\n";
#endif
  }

  return stations;
}

double StationGuesser::stationHeuristic(double similarity,
                                        int stationIndex) const
{
  return 800 * similarity + _stationSizes[stationIndex];
}

void StationGuesser::normalize(std::string& s)
{
  boost::replace_all(s, "Ä", "a");
  boost::replace_all(s, "ä", "a");
  boost::replace_all(s, "Ö", "o");
  boost::replace_all(s, "ö", "o");
  boost::replace_all(s, "Ü", "u");
  boost::replace_all(s, "ü", "u");
  boost::replace_all(s, "ß", "ss");
  boost::replace_all(s, "-", " ");
  boost::replace_all(s, "/", " ");
  boost::replace_all(s, ".", " ");
  boost::replace_all(s, ",", " ");
  boost::replace_all(s, "(", " ");
  boost::replace_all(s, ")", " ");
  boost::replace_all(s, "  ", " ");
  std::transform(s.begin(), s.end(), s.begin(), ::tolower);
}

double StationGuesser::similarity(int stationIndex,
                                  const std::vector<std::string>& tRef) const
{
  const std::vector<std::string>& tCan = _stationTrigrams[stationIndex];

  int matchCount = 0;
  for (const std::string& t : tRef)
    if (std::find(tCan.begin(), tCan.end(), t) != tCan.end())
      ++matchCount;

  double match_percent = static_cast<double>(matchCount) / tRef.size();
  double coverage = static_cast<double>(tCan.size()) / tRef.size();
  double cov_distance = std::abs(coverage - 1.0);

  return (match_percent * 4 - cov_distance) / 4.0;
}

std::vector<std::string> StationGuesser::trigrams(const std::string& input)
{
  if (input.length() < 3)
    return std::vector<std::string>();

  std::string expanded = std::string("  ") + input + " ";
  std::vector<std::string> tri(expanded.length() - 2);
  for (unsigned i = 0; i < expanded.length() - 2; ++i)
    tri[i] = expanded.substr(i, 3);

  return tri;
}

std::string StationGuesser::build_regex(const std::string& s)
{
  std::size_t stopIndex = std::string::npos;

  std::size_t whiteSpacePos = s.substr(0, TRIM).find(" ");
  if (whiteSpacePos != std::string::npos)
    stopIndex = whiteSpacePos;
  else
    stopIndex = std::min(s.length() - 1, static_cast<std::size_t>(TRIM));

  std::string re = "(^|.*\\W)";
  for (int i = 1; i < static_cast<int>(stopIndex) + 1; ++i)
  {
    int start = std::max(0, i - 1);
    int end = std::min(static_cast<std::size_t>(i + 1), s.length());
    re = re +  "[" + s.substr(start, end - start) + "]";
    if (i != static_cast<int>(stopIndex))
      re += ".?";
  }
  re += ".*";

  return re;
}
