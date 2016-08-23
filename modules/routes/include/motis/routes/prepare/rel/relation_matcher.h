#pragma once

#include <vector>

#include "motis/core/common/geo.h"
#include "motis/routes/prepare/point_rtree.h"

#include "motis/schedule-format/Schedule_generated.h"

namespace motis {
namespace routes {

struct station_seq {
  friend bool operator<(station_seq const& lhs, station_seq const& rhs) {
    return std::tie(lhs.station_ids_, lhs.coordinates_) <
           std::tie(rhs.station_ids_, rhs.coordinates_);
  }

  friend bool operator==(station_seq const& lhs, station_seq const& rhs) {
    return std::tie(lhs.station_ids_, lhs.coordinates_) ==
           std::tie(rhs.station_ids_, rhs.coordinates_);
  }

  std::vector<std::string> station_ids_;
  std::vector<geo_detail::latlng> coordinates_;
};

std::vector<station_seq> load_station_sequences(
    motis::loader::Schedule const* sched) {
  std::vector<station_seq> result;

  for (auto const& route : *sched->routes()) {
    station_seq seq;

    for (auto const& station : *route->stations()) {
      seq.station_ids_.emplace_back(station->id()->str());
      seq.station_ids_.emplace_back(station->lat(), station->lng());
    }
    result.emplace_back(std::move(seq));
  }

  std::sort(begin(result), end(result));
  result.erase(std::unique(begin(result), end(result)), end(result));

  return result;
}

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
