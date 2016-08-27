#pragma once

#include <vector>

#include "motis/core/common/geo.h"
#include "motis/routes/prepare/point_rtree.h"

#include "motis/schedule-format/Schedule_generated.h"

namespace motis {
namespace routes {

void match_sequences(
    std::vector<std::vector<geo_detail::latlng>> const& polylines,
    std::vector<station_seq> const& sequences) {

  for (auto const& polyline : polylines) {
    auto rtree = make_point_rtree(polyline, [&](auto&& c) {
      return point_rtree::point{c.lng_, c.lat_};
    });

    for (auto const& seq : sequences) {
      for (auto i = 0u; i < seq.coordinates_.size(); ++i) {
        auto close_nodes = rtree.in_radius(seq.coordinates_[i].lat_,
                                           seq.coordinates_[i].lng_, 50);

      }
    }
  }

  // auto seq = load
}

}  // namespace routes
}  // namespace motis
