#include "motis/routes/prepare/rel/relation_matcher.h"

#include "motis/core/common/logging.h"

namespace motis {
namespace routes {

std::vector<std::vector<match_seq>> match_sequences(
    std::vector<std::vector<latlng>> const& polylines,
    std::vector<station_seq> const& sequences,
    std::map<std::string, std::vector<latlng>> const& bus_stops) {
  std::vector<std::vector<match_seq>> result(sequences.size());

  for (auto const& polyline : polylines) {
    auto rtree = make_point_rtree(polyline, [&](auto&& c) {
      return point_rtree::point{c.lng_, c.lat_};
    });
    for (auto i = 0u; i < sequences.size(); ++i) {
      auto matches =
          matches_on_seq(sequences[i], rtree, polyline, bus_stops, 100);
      result[i].insert(end(result[i]), begin(matches), end(matches));
    }
  }
  return result;
}

std::vector<match_seq> matches_on_seq(
    station_seq const& seq, point_rtree const& rtree,
    std::vector<latlng> const& polyline,
    std::map<std::string, std::vector<latlng>> const& bus_stops,
    int const& radius) {

  std::vector<match_seq> matches;
  long last = -1;
  long first = -1;

  std::vector<std::pair<size_t, size_t>> stations;
  for (auto i = 0u; i < seq.coordinates_.size(); ++i) {
    auto close_nodes = rtree.in_radius(seq.coordinates_[i].lat_,
                                       seq.coordinates_[i].lng_, radius);

    auto bus_nodes = bus_stops.find(seq.station_ids_[i]);

    if (bus_nodes != end(bus_stops)) {
      std::for_each(
          begin(bus_nodes->second), end(bus_nodes->second), [&](auto&& stop) {
            auto close_bus_nodes =
                rtree.in_radius(stop.lat_, stop.lng_, radius);
            close_nodes.insert(begin(close_nodes), begin(close_bus_nodes),
                               end(close_bus_nodes));
          });
    }

    if (close_nodes.empty()) {
      try_add_match(first, last, polyline, stations, false, matches);
      first = -1;
      continue;
    }

    long node = *std::min_element(begin(close_nodes), end(close_nodes));
    if (node <= last) {
      break;
    }
    if (first == -1) {
      first = node;
    }
    stations.emplace_back(i, node - first);

    last = node;

    if (i == seq.coordinates_.size() - 1 && stations.size() == i) {
      try_add_match(first, last, polyline, stations, true, matches);
    }
  }

  return matches;
}

void try_add_match(long first, long last, std::vector<latlng> const& polyline,
                   std::vector<std::pair<size_t, size_t>>& stations,
                   bool full_match, std::vector<match_seq>& matches) {
  if (stations.size() < 2) {
    stations.clear();
    return;
  }
  match_seq match;
  match.stations_ = stations;
  match.polyline_.insert(begin(match.polyline_), begin(polyline) + first,
                         begin(polyline) + last + 1);
  match.full_match_ = full_match;
  matches.emplace_back(std::move(match));
  stations.clear();
}

}  // namespace motis
}  // namespace routes
