#pragma once

#include <vector>

#include "motis/core/common/geo.h"

#include "motis/routes/prepare/station_sequences.h"

namespace motis {
namespace routes {

struct match_seq {
  bool full_match_ = false;
  std::vector<latlng> polyline_;
  std::vector<std::pair<size_t, size_t>> stations_;
};

std::vector<std::vector<match_seq>> match_sequences(
    std::vector<std::vector<latlng>> const& polylines,
    std::vector<station_seq> const& sequences,
    std::map<std::string, std::vector<latlng>> const& bus_stops);

}  // namespace routes
}  // namespace motis
