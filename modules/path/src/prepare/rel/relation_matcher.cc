#include "motis/path/prepare/rel/relation_matcher.h"

#include <mutex>

#include "utl/concat.h"
#include "utl/erase_if.h"
#include "utl/parallel_for.h"

#include "geo/point_rtree.h"

#include "motis/core/common/logging.h"
#include "motis/core/common/transform_to_vec.h"

#include "motis/path/prepare/rel/polyline_aggregator.h"
#include "motis/path/prepare/rel/relation_parser.h"

using namespace geo;
using namespace motis::logging;

namespace motis {
namespace path {

constexpr auto kMatchRadius = 200;

struct matcher {
  matcher(aggregated_polyline const& ap, station_seq const& seq,
          std::map<std::string, std::vector<latlng>> const& stop_positions,
          point_rtree const& rtree)
      : ap_(ap), seq_(seq), stop_positions_(stop_positions), rtree_(rtree) {}

  void run() {
    long last = -1;
    long first = -1;
    std::vector<std::pair<size_t, size_t>> stations;

    for (auto i = 0u; i < seq_.coordinates_.size(); ++i) {
      auto close_nodes =
          rtree_.in_radius_with_distance(seq_.coordinates_[i], kMatchRadius);

      auto const& extra_nodes = stop_positions_.find(seq_.station_ids_[i]);
      if (extra_nodes != end(stop_positions_)) {
        for (auto const& stop_pos : extra_nodes->second) {
          utl::concat(close_nodes,
                 rtree_.in_radius_with_distance(stop_pos, kMatchRadius));
        }
      }

      utl::erase_if(close_nodes, [&](auto const& pair) {
        return static_cast<long>(pair.second) <= last;
      });

      if (close_nodes.empty()) {
        finish_match(first, last, stations);
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

      if (i == seq_.coordinates_.size() - 1 && stations.size() == i) {
        finish_match(first, last, stations);
      }
    }

    finish_match(first, last, stations);
  }

  void finish_match(long first, long last,
                    std::vector<std::pair<size_t, size_t>>& stations) {
    if (stations.size() < 2) {
      stations.clear();
      return;
    }
    match_seq match;
    match.source_ = ap_.source_;
    match.stations_ = stations;
    match.polyline_.insert(begin(match.polyline_), begin(ap_.polyline_) + first,
                           begin(ap_.polyline_) + last + 1);
    matches_.emplace_back(std::move(match));
    stations.clear();
  }

  aggregated_polyline const& ap_;
  station_seq const& seq_;
  std::map<std::string, std::vector<latlng>> const& stop_positions_;

  point_rtree const& rtree_;

  std::vector<match_seq> matches_;
};

std::vector<std::vector<match_seq>> match_sequences(
    std::vector<aggregated_polyline> const& aps,
    std::vector<station_seq> const& sequences,
    std::map<std::string, std::vector<latlng>> const& stop_positions) {
  std::vector<std::vector<match_seq>> result(sequences.size());

  std::mutex m;
  utl::parallel_for("match sequences", aps, 250, [&](auto const& ap) {
    auto const rtree = make_point_rtree(ap.polyline_);

    auto const matches = transform_to_vec(
        sequences, [&](auto const& seq) -> std::vector<match_seq> {
          if (std::none_of(
                  begin(seq.categories_), end(seq.categories_),
                  [&](auto const& c) { return ap.source_.category_ == c; })) {
            return {};
          }

          auto m = matcher{ap, seq, stop_positions, rtree};
          m.run();
          return m.matches_;
        });

    std::lock_guard<std::mutex> lock(m);
    for (auto i = 0u; i < matches.size(); ++i) {
      utl::concat(result[i], matches[i]);
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
}  // namespace path
