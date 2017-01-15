#pragma once

#include <type_traits>
#include <vector>

#include "geo/latlng.h"
#include "geo/point_rtree.h"

#include "utl/to_vec.h"

namespace motis {
namespace path {

struct rail_node_phantom {
  rail_node_phantom() = default;
  rail_node_phantom(int64_t const id, geo::latlng const pos)
      : id_(id), pos_(pos) {}

  std::vector<size_t> way_idx_;
  int64_t id_;
  geo::latlng pos_;
};

struct rail_edge_phantom {
  rail_edge_phantom(size_t const way_idx, size_t const offset,
                    geo::latlng const pos)
      : way_idx_(way_idx), offset_(offset), pos_(pos) {}
  size_t way_idx_;
  size_t offset_;
  geo::latlng pos_;
};

bool way_idx_match(std::vector<size_t> const& vec, size_t const val) {
  return std::find(begin(vec), end(vec), val) != end(vec);
}

bool way_idx_match(size_t const val, std::vector<size_t> const& vec) {
  return way_idx_match(vec, val);
}

struct rail_phantom_index {
  rail_phantom_index(std::vector<rail_way> const& rail_ways) {
    hash_map<int64_t, rail_node_phantom> node_phantoms;
    node_phantoms.set_empty_key(std::numeric_limits<int64_t>::max());

    for (auto i = 0u; i < rail_ways.size(); ++i) {
      auto const& polyline = rail_ways[i].polyline_;
      verify(polyline.size() > 1, "rail: polyline too short");

      auto const make_node_phantom = [&](auto const id, auto const pos) {
        auto& phantom = map_get_or_create(node_phantoms, id, [&] {
          return rail_node_phantom{id, pos};
        });
        phantom.way_idx_.push_back(i);
      };
      make_node_phantom(rail_ways[i].from_, polyline.front());
      make_node_phantom(rail_ways[i].to_, polyline.back());

      for (auto j = 1u; j < polyline.size() - 1; ++j) {
        edge_phantoms_.emplace_back(i, j, polyline[j]);
      }
    }

    node_phantoms_ = utl::to_vec(node_phantoms,
                                 [](auto const& pair) { return pair.second; });

    node_phantom_rtree_ = geo::make_point_rtree(
        node_phantoms_, [](auto const& p) { return p.pos_; });

    edge_phantom_rtree_ = geo::make_point_rtree(
        edge_phantoms_, [](auto const& p) { return p.pos_; });
  }

  std::pair<std::vector<rail_node_phantom const*>,
            std::vector<rail_edge_phantom const*>>
  get_rail_phantoms(geo::latlng const& pos, double const radius) const {
    auto const nodes =
        utl::to_vec(node_phantom_rtree_.in_radius(pos, radius),
                    [&](auto const& idx) { return &node_phantoms_[idx]; });

    std::set<size_t> ways;
    std::vector<rail_edge_phantom const*> edges;
    for (auto const& idx : edge_phantom_rtree_.in_radius(pos, radius)) {
      auto& phantom = edge_phantoms_[idx];
      if (ways.find(phantom.way_idx_) != end(ways)) {
        continue;  // rtree: already sorted by distance
      }

      ways.insert(phantom.way_idx_);
      edges.push_back(&phantom);
    }

    // retain only of no neighbor is better
    auto const filter = [&pos](auto const& subjects, auto const& others) {
      typename std::decay<decltype(subjects)>::type result;
      std::copy_if(begin(subjects), end(subjects), std::back_inserter(result),
                   [&pos, &others](auto const* s) {
                     return std::none_of(
                         begin(others), end(others), [&pos, &s](auto const* o) {
                           return way_idx_match(o->way_idx_, s->way_idx_) &&
                                  geo::distance(pos, o->pos_) <
                                      geo::distance(pos, s->pos_);
                         });
                   });
      return result;
    };

    return {filter(nodes, edges), filter(edges, nodes)};
  }

  std::vector<rail_node_phantom> node_phantoms_;
  geo::point_rtree node_phantom_rtree_;

  std::vector<rail_edge_phantom> edge_phantoms_;
  geo::point_rtree edge_phantom_rtree_;
};

}  // namespace path
}  // namespace motis
