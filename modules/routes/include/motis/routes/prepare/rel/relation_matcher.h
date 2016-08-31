#pragma once

#include <vector>

#include "motis/core/common/geo.h"
#include "motis/core/common/logging.h"
#include "motis/routes/prepare/point_rtree.h"
#include "motis/routes/prepare/station_sequences.h"

#include "motis/schedule-format/Schedule_generated.h"

namespace motis {
namespace routes {

struct match_seq {

  match_seq(std::vector<latlng> polyline, std::vector<std::string> station_ids)
      : polyline_(polyline), station_ids_(station_ids){};

  bool full_match = false;
  std::vector<latlng> polyline_;
  std::vector<std::string> station_ids_;
};

std::vector<match_seq> match_sequences(
    std::vector<std::vector<latlng>> const& polylines,
    std::vector<station_seq> const& sequences) {
  std::vector<match_seq> result;
  logging::scoped_timer("Matching polylines");
  for (auto const& polyline : polylines) {
    auto rtree = make_point_rtree(polyline, [&](auto&& c) {
      return point_rtree::point{c.lng_, c.lat_};
    });

    for (auto const& seq : sequences) {
      std::vector<std::string> station_ids;
      for (auto i = 0u; i < seq.coordinates_.size(); ++i) {
        auto close_nodes = rtree.in_radius(seq.coordinates_[i].lat_,
                                           seq.coordinates_[i].lng_, 50);
        auto next_node = std::distance(
            begin(close_nodes),
            std::min_element(begin(close_nodes), end(close_nodes)));
        if (close_nodes.empty()) {
          station_ids.clear();
          continue;
        }
        station_ids.push_back(seq.station_ids_[i]);
        if ((i == seq.coordinates_.size() - 1) && station_ids.size() >= 2) {
          match_seq match(polyline, station_ids);
          if (station_ids.size() == seq.station_ids_.size()) {
            match.full_match = true;
          }
          result.push_back(std::move(match));
        }
      }
    }
  }
  return result;
}

}  // namespace routes
}  // namespace motis
