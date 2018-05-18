#pragma once

#include <string>
#include <vector>

#include "geo/latlng.h"

#include "parser/util.h"

#include "utl/erase_duplicates.h"

#include "motis/core/common/logging.h"

#include "motis/path/prepare/schedule/station_sequences.h"

namespace motis {
namespace path {

struct station {
  station(std::string id, geo::latlng pos) : id_(std::move(id)), pos_(pos) {}

  friend bool operator<(station const& lhs, station const& rhs) {
    return std::tie(lhs.id_) < std::tie(rhs.id_);
  }

  friend bool operator==(station const& lhs, station const& rhs) {
    return std::tie(lhs.id_) == std::tie(rhs.id_);
  }

  std::string id_;
  geo::latlng pos_;
};

inline std::vector<station> all_stations(std::vector<station_seq> const& seqs) {
  std::vector<station> stations;
  for (auto const& seq : seqs) {
    verify(seq.station_ids_.size() == seq.coordinates_.size(), "invalid seq!");

    for (auto i = 0u; i < seq.station_ids_.size(); ++i) {
      stations.emplace_back(seq.station_ids_[i], seq.coordinates_[i]);
    }
  }

  utl::erase_duplicates(stations);
  return stations;
}

}  // namespace path
}  // namespace motis
