#include "motis/routes/prepare/rel/polyline_aggregator.h"

#include <iostream>
#include <limits>
#include <set>

#include "boost/optional.hpp"

using namespace motis::geo;

namespace motis {
namespace routes {

constexpr auto kIdentity = -std::numeric_limits<double>::infinity();
constexpr auto kUnreachable = std::numeric_limits<double>::infinity();
constexpr auto kMaxFuzzyDistance = 10.;

std::vector<std::vector<double>> distance_matrix(
    std::vector<way*> const& ways) {
  auto distance = [](node const& from, node const& to) {
    if (from.id_ == to.id_) {
      return kIdentity;
    }

    double const dist = geo::distance(from.pos_, to.pos_);
    if (dist < kMaxFuzzyDistance) {
      return dist;
    }

    return kUnreachable;
  };

  std::vector<std::vector<double>> distances;
  for (auto i = 0u; i < ways.size(); ++i) {
    std::vector<double> head_distances;
    std::vector<double> tail_distances;

    auto const from_head = ways[i]->head();
    auto const from_tail = ways[i]->tail();

    for (auto j = 0u; j < ways.size(); ++j) {
      if (i == j) {
        head_distances.push_back(kUnreachable);
        head_distances.push_back(kUnreachable);

        tail_distances.push_back(kUnreachable);
        tail_distances.push_back(kUnreachable);
        continue;
      }

      auto const to_head = ways[j]->head();
      auto const to_tail = ways[j]->tail();

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

    } else if (distances[i] != kUnreachable) {
      dist_matches.push_back(i);
    }
  }

  if (id_match) {
    return id_match;
  } else if (dist_matches.size() == 1) {
    return dist_matches[0];
  }

  return {};
}

void append(std::vector<latlng>& result, way const* w, bool const forward) {
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

std::pair<std::vector<latlng>, size_t> extract_polyline(
    std::vector<way*> const& ways,
    std::vector<std::vector<double>> const& distances, size_t start_idx,
    bool start_forward) {
  size_t idx = start_idx;
  bool forward = start_forward;

  std::vector<latlng> result;
  while (true) {
    append(result, ways[idx], forward);

    auto next = find_next(distances[2 * idx + (forward ? 0 : 1)]);
    if (!next) {
      return {std::move(result), idx};
    }

    idx = *next / 2;
    forward = (*next % 2) != 0;
  }

  assert(false);  // !?
}

void aggregate_ways(std::vector<std::vector<latlng>>& polylines,
                    std::set<size_t>& visited, std::vector<way*> const& ways,
                    std::vector<std::vector<double>> const& matrix,
                    bool forward) {
  for (auto i = 0u; i < ways.size(); ++i) {
    auto const& dists = matrix[2 * i + (forward ? 1 : 0)];
    if (std::any_of(begin(dists), end(dists),
                    [](auto&& d) { return d != kUnreachable; })) {
      continue;
    }

    if (visited.find(i) != end(visited)) {
      continue;
    }

    visited.insert(i);

    auto result = extract_polyline(ways, matrix, i, true);

    if (i == result.second) {
      continue;  // a single way is usually bullshit
    }

    visited.insert(result.second);
    polylines.emplace_back(std::move(result.first));
  }
}

std::vector<std::vector<latlng>> aggregate_polylines(
    std::vector<relation> relations) {

  std::vector<std::vector<latlng>> polylines;
  for (auto& relation : relations) {
    auto& ways = relation.ways_;

    for (auto& way : ways) {
      for (auto& node : way->nodes_) {
        if (!node.resolved_) {
          std::cout << "missing node" << node.id_ << std::endl;
        }
      }
    }

    ways.erase(std::remove_if(begin(ways), end(ways),
                              [](auto const& w) {
                                return !w->resolved_ || w->nodes_.size() < 2;
                              }),
               end(ways));

    auto dists = distance_matrix(ways);
    auto visited = std::set<size_t>{};

    aggregate_ways(polylines, visited, ways, dists, true);
    aggregate_ways(polylines, visited, ways, dists, false);
  }

  return polylines;
}

}  // namespace routes
}  // namespace motis
