#pragma once

#include <set>
#include <vector>

#include "geo/latlng.h"

namespace motis {

namespace loader {
struct Schedule;
}  // namespace loader

namespace path {

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
  std::vector<geo::latlng> coordinates_;

  std::set<int> categories_;
  std::set<int> train_nrs_;
};

std::vector<station_seq> load_station_sequences(motis::loader::Schedule const*);

}  // namespace path
}  // namespace motis
