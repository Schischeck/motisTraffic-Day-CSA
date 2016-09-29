#include "motis/routes/prepare/rel/relation_matcher.h"

#include <mutex>

#include "motis/core/common/logging.h"

#include "motis/routes/prepare/parallel_for.h"
#include "motis/routes/prepare/rel/polyline_aggregator.h"
#include "motis/routes/prepare/rel/relation_parser.h"
#include "motis/routes/prepare/vector_utils.h"

using namespace motis::geo;
using namespace motis::logging;

namespace motis {
namespace routes {

void finish_match(long first, long last, std::vector<latlng> const& polyline,
                  std::vector<std::pair<size_t, size_t>>& stations,
                  std::vector<match_seq>& matches) {
  if (stations.size() < 2) {
    stations.clear();
    return;
  }
  match_seq match;
  match.stations_ = stations;
  match.polyline_.insert(begin(match.polyline_), begin(polyline) + first,
                         begin(polyline) + last + 1);
  matches.emplace_back(std::move(match));
  stations.clear();
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
    auto close_nodes = rtree.in_radius_with_distance(
        seq.coordinates_[i].lat_, seq.coordinates_[i].lng_, radius);

    auto const& bus_nodes = bus_stops.find(seq.station_ids_[i]);
    if (bus_nodes != end(bus_stops)) {
      for (auto const& stop_pos : bus_nodes->second) {
        append(close_nodes, rtree.in_radius_with_distance(
                                stop_pos.lat_, stop_pos.lng_, radius));
      }
    }

    erase_if(close_nodes, [&](auto const& pair) {
      return static_cast<long>(pair.second) <= last;
    });

    if (close_nodes.empty()) {
      finish_match(first, last, polyline, stations, matches);
      first = -1;
      continue;
    }

    auto const node =
        std::min_element(begin(close_nodes), end(close_nodes))->second;
    if (first == -1) {
      first = node;
    }
    stations.emplace_back(i, node - first);

    last = node;

    if (i == seq.coordinates_.size() - 1 && stations.size() == i) {
      finish_match(first, last, polyline, stations, matches);
    }
  }

  finish_match(first, last, polyline, stations, matches);

  return matches;
}

std::vector<std::vector<match_seq>> match_sequences(
    std::vector<aggregated_polyline> const& aps,
    std::vector<station_seq> const& sequences,
    std::map<std::string, std::vector<latlng>> const& bus_stops) {
  std::vector<std::vector<match_seq>> result(sequences.size());

  std::mutex m;
  parallel_for("match_polyline", aps, 250, [&](auto const& ap) {
    auto rtree = make_point_rtree(ap.polyline_, [&](auto&& c) {
      return point_rtree::point{c.lng_, c.lat_};
    });

    auto const matches = transform_to_vec(sequences, [&](auto const& seq) {
      return matches_on_seq(seq, rtree, polyline, bus_stops, 200);
    });

    std::lock_guard<std::mutex> lock(m);
    for (auto i = 0u; i < matches.size(); ++i) {
      append(result[i], matches[i]);
    }
  });

  return result;
}

std::vector<std::vector<match_seq>> match_osm_relations(
    std::string const& osm_file, std::vector<station_seq> const& sequences,
    std::map<std::string, std::vector<geo::latlng>> const&
        additional_stop_positions) {
  auto const relations = parse_relations(osm_file);
  LOG(info) << "found " << relations.relations_.size() << " relations";

  auto const polylines = aggregate_polylines(relations.relations_);

  return match_sequences(polylines, sequences, additional_stop_positions);
};

}  // namespace motis
}  // namespace routes
