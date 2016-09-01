#pragma once

#include <vector>

#include "motis/core/common/geo.h"

#include "motis/routes/prepare/station_sequences.h"

namespace motis {
namespace routes {

struct match_seq {
  match_seq(){};

  match_seq(std::vector<std::string> station_ids, std::vector<latlng> polyline)
      : polyline_(polyline), station_ids_(station_ids){};

  friend bool operator==(match_seq const& lhs, match_seq const& rhs) {
    return std::tie(lhs.station_ids_, lhs.polyline_) ==
           std::tie(rhs.station_ids_, rhs.polyline_);
  }

  friend bool operator<(match_seq const& lhs, match_seq const& rhs) {
    return std::tie(lhs.station_ids_, lhs.polyline_) <
           std::tie(rhs.station_ids_, rhs.polyline_);
  }

  bool full_match = false;
  std::vector<latlng> polyline_;
  std::vector<std::string> station_ids_;
};

std::vector<match_seq> match_sequences(
    std::vector<std::vector<latlng>> const& polylines,
    std::vector<station_seq> const& sequences,
    std::map<std::string, std::vector<latlng>> const& bus_stops);

std::vector<match_seq> match_better(std::vector<station_seq> const& sequences,
                                    std::vector<match_seq>& matches);

}  // namespace routes
}  // namespace motis
