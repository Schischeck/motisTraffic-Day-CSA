#pragma once

#include <limits>
#include <string>
#include <vector>
#include <unordered_map>

#include "geo/latlng.h"

#include "motis/core/common/hash_helper.h"
#include "motis/core/common/hash_map.h"

#include "motis/module/message.h"

namespace motis {
namespace routes {

struct seq_key {
  seq_key() = default;
  seq_key(std::vector<std::string> station_ids, uint32_t clasz)
      : station_ids_(std::move(station_ids)), clasz_(clasz) {}

  friend bool operator==(seq_key const& lhs, seq_key const& rhs) {
    return std::tie(lhs.station_ids_, lhs.clasz_) ==
           std::tie(rhs.station_ids_, rhs.clasz_);
  }

  std::vector<std::string> station_ids_;
  uint32_t clasz_;
};

}  // namespace routes
}  // namespace motis

namespace std {

template <>
struct hash<motis::routes::seq_key> {
  std::size_t operator()(motis::routes::seq_key const& k) const {
    std::size_t seed = 0;
    for (auto const& id : k.station_ids_) {
      motis::hash_combine(seed, id);
    }
    motis::hash_combine(seed, k.clasz_);
    return seed;
  }
};

}  // namespace std

namespace motis {
namespace routes {

struct auxiliary_data {

  auxiliary_data() {
    extra_stop_positions_.set_empty_key(std::string{""});
    // prepared_routes_.set_empty_key(
    //     seq_key{{}, std::numeric_limits<uint32_t>::max()});
  }

  void load(std::string const& filename);

  hash_map<std::string, std::vector<geo::latlng>> extra_stop_positions_;
  // hash_map<seq_key, motis::module::msg_ptr> prepared_routes_;
  std::unordered_map<seq_key, motis::module::msg_ptr> prepared_routes_;
};

}  // namespace routes
}  // namespace motis
