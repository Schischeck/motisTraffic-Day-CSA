#include "motis/routes/prepare/rel/polyline_aggregator.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <mutex>
#include <set>

#include "boost/optional.hpp"
#include "geo/latlng.h"

#include "motis/core/common/logging.h"

#include "motis/routes/prepare/geojson.h"
#include "motis/routes/prepare/parallel_for.h"
#include "motis/routes/prepare/vector_utils.h"

using namespace geo;
using namespace motis::logging;

namespace motis {
namespace routes {

constexpr auto kIdentity = -std::numeric_limits<double>::infinity();
constexpr auto kUnreachable = std::numeric_limits<double>::infinity();
constexpr auto kInvalidTurn = std::numeric_limits<double>::max();

constexpr auto kMaxFuzzyDistance = 10.;
constexpr auto kMinFuzzyAngle = 45.;

struct endpoint {
  enum type : bool { HEAD /* = back() */, TAIL /* = front() */ };

  endpoint(way const* w, type const t) {
    if (t == HEAD) {
      outer_ = &w->nodes_[w->nodes_.size() - 1];
      inner_ = &w->nodes_[w->nodes_.size() - 2];
    } else {
      outer_ = &w->nodes_[0];
      inner_ = &w->nodes_[1];
    }

    bearing_ = bearing(outer_->pos_, inner_->pos_);
  }

  node const* outer_;
  node const* inner_;

  double bearing_;
};

double distance(endpoint const& from, endpoint const& to) {
  if (from.outer_->id_ == to.outer_->id_) {
    return kIdentity;
  }

  double const dist = geo::distance(from.outer_->pos_, to.outer_->pos_);
  if (dist < kMaxFuzzyDistance) {
    // see http://stackoverflow.com/a/7869457
    auto angle = std::abs(
        std::fmod(std::abs(from.bearing_ - to.bearing_ + 180.), 360.) - 180.);
    if (angle < kMinFuzzyAngle) {
      return kInvalidTurn;
    }
    return dist;
  }

  return kUnreachable;
};

std::vector<std::vector<double>> distance_matrix(
    std::vector<way*> const& ways) {
  std::vector<std::vector<double>> distances;
  for (auto i = 0u; i < ways.size(); ++i) {
    std::vector<double> head_distances;
    std::vector<double> tail_distances;

    auto const from_head = endpoint{ways[i], endpoint::HEAD};
    auto const from_tail = endpoint{ways[i], endpoint::TAIL};

    for (auto j = 0u; j < ways.size(); ++j) {
      if (i == j) {
        head_distances.push_back(kUnreachable);
        head_distances.push_back(kUnreachable);

        tail_distances.push_back(kUnreachable);
        tail_distances.push_back(kUnreachable);
        continue;
      }

      auto const to_head = endpoint{ways[j], endpoint::HEAD};
      auto const to_tail = endpoint{ways[j], endpoint::TAIL};

      head_distances.push_back(distance(from_head, to_head));
      head_distances.push_back(distance(from_head, to_tail));

      tail_distances.push_back(distance(from_tail, to_head));
      tail_distances.push_back(distance(from_tail, to_tail));
    }

    distances.emplace_back(std::move(head_distances));
    distances.emplace_back(std::move(tail_distances));
  }

  return distances;
}

boost::optional<size_t> find_next(std::vector<double> const& distances) {
  boost::optional<size_t> id_match;
  std::vector<size_t> dist_matches;

  for (auto i = 0u; i < distances.size(); ++i) {
    if (distances[i] == kIdentity) {
      if (id_match) {
        return {};  // more than one id match -> abort
      }
      id_match = i;

    } else if (distances[i] < kUnreachable) {
      dist_matches.push_back(i);
    }
  }

  if (id_match) {
    return id_match;
  } else if (dist_matches.size() == 1 &&
             distances[dist_matches[0]] < kMaxFuzzyDistance) {
    return dist_matches[0];
  }

  return {};
}

void append(polyline& result, way const* w, bool const forward) {
  if (forward) {
    for (auto const& n : w->nodes_) {
      result.push_back(n.pos_);
    }
  } else {
    for (long i = static_cast<int>(w->nodes_.size()) - 1; i >= 0; --i) {
      result.push_back(w->nodes_[i].pos_);
    }
  }
}

size_t extract_polyline(polyline& line, std::set<size_t>& visited,
                        std::vector<way*> const& ways,
                        std::vector<std::vector<double>> const& distances,
                        size_t start_idx, bool start_forward) {
  size_t idx = start_idx;
  bool forward = start_forward;

  size_t count = 0;
  while (auto const next = find_next(distances[2 * idx + (forward ? 0 : 1)])) {
    idx = *next / 2;
    forward = (*next % 2) != 0;
    if (std::find(begin(visited), end(visited), idx) != std::end(visited)) {
      break;
    }
    append(line, ways[idx], forward);
    visited.insert(idx);
    ++count;
  }

  return count;
}

template <typename Fun>
void aggregate_ways(Fun appender, std::set<size_t>& visited,
                    std::vector<way*> const& ways,
                    std::vector<std::vector<double>> const& matrix,
                    bool forward) {
  for (auto i = 0u; i < ways.size(); ++i) {
    auto const& dists = matrix[2 * i + (forward ? 1 : 0)];
    if (std::any_of(begin(dists), end(dists),
                    [](auto&& d) { return d < kMaxFuzzyDistance; })) {
      continue;
    }

    if (visited.find(i) != end(visited)) {
      continue;
    }

    polyline p;
    append(p, ways[i], true);

    if ((extract_polyline(p, visited, ways, matrix, i, true) +
         extract_polyline(p, visited, ways, matrix, i, false)) == 0) {
      continue;  // a single way is usually not helpful
    }

    p.erase(std::unique(begin(p), end(p)), end(p));
    appender(std::move(p));
  }
}

std::vector<aggregated_polyline> aggregate_polylines(
    std::vector<relation> const& relations) {
  scoped_timer timer("aggregate polylines");

  std::mutex m;
  std::vector<aggregated_polyline> polylines;

  parallel_for("aggregate polylines", relations, 250, [&](relation const& rel) {
    auto dists = distance_matrix(rel.ways_);
    auto visited = std::set<size_t>{};

    auto appender = [&](polyline&& p) {
      std::lock_guard<std::mutex> lock(m);
      polylines.emplace_back(rel.source_, std::move(p));
    };

    aggregate_ways(appender, visited, rel.ways_, dists, true);
    aggregate_ways(appender, visited, rel.ways_, dists, false);
  });

  return polylines;
}

}  // namespace routes
}  // namespace motis
