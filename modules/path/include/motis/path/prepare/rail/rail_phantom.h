#pragma once

#include <vector>

#include "geo/latlng.h"
#include "geo/point_rtree.h"

#include "utl/to_vec.h"

namespace motis {
namespace path {

struct rail_phantom {
  rail_phantom(size_t const info_idx, size_t const point_idx, size_t const dist,
               geo::latlng const pos)
      : info_idx_(info_idx), point_idx_(point_idx), dist_(dist), pos_(pos) {}

  size_t info_idx_;
  size_t point_idx_;

  size_t dist_;  // from start
  geo::latlng pos_;
};

struct rail_phantom_index {
  rail_phantom_index(rail_graph const& graph) {
    std::vector<geo::latlng> coords;
    for (auto i = 0u; i < graph.infos_.size(); ++i) {
      auto const& polyline = graph.infos_[i]->polyline_;

      size_t dist = 0;
      for (auto j = 0u; j < polyline.size(); ++j) {
        coords.push_back(polyline[j]);

        if (j > 0) {
          dist += geo::distance(polyline[j - 1], polyline[j]);
        }

        nodes_.emplace_back(i, j, dist, polyline[j]);
      }
    }

    rtree_ = geo::make_point_rtree(coords);
  }

  std::vector<rail_phantom> get_rail_phantoms(geo::latlng const& pos,
                                              double const radius) const {
    // unique!
    return utl::to_vec(rtree_.in_radius(pos, radius),
                       [&](auto const& idx) { return nodes_[idx]; });
  }

  std::vector<rail_phantom> nodes_;
  geo::point_rtree rtree_;
};

}  // namespace path
}  // namespace motis
