#pragma once

#include <vector>

#include "motis/core/common/geo.h"

namespace motis {

namespace loader {
struct Schedule;
}  // namespace loader

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

std::vector<station_seq> load_station_sequences(motis::loader::Schedule const*);

}  // namespace routes
}  // namespace motis
