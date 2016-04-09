#pragma once

#include <cinttypes>

#include <vector>
#include <utility>

#include "motis/core/schedule/edges.h"

namespace motis {

struct primary_trip_id {
  primary_trip_id() = default;
  primary_trip_id(uint32_t station_id, uint32_t train_nr, motis::time time)
      : station_id(station_id), train_nr(train_nr), time(time) {}

  uint64_t station_id : 31;
  uint64_t train_nr : 17;
  uint64_t time : 16;

  friend bool operator<(primary_trip_id const& lhs,
                        primary_trip_id const& rhs) {
    return *reinterpret_cast<uint64_t const*>(&lhs) <
           *reinterpret_cast<uint64_t const*>(&rhs);
  }

  friend bool operator==(primary_trip_id const& lhs,
                         primary_trip_id const& rhs) {
    return *reinterpret_cast<uint64_t const*>(&lhs) ==
           *reinterpret_cast<uint64_t const*>(&rhs);
  }
};

}  // namespace motis

namespace std {
template <>
struct hash<motis::primary_trip_id> {
  std::size_t operator()(motis::primary_trip_id const& e) const {
    return *reinterpret_cast<uint64_t const*>(&e);
  }
};
}  // namespace std

namespace motis {

struct secondary_trip_id {
  secondary_trip_id() = default;
  secondary_trip_id(uint32_t target_station_id, uint16_t target_time,
                    bool is_arrival, std::string line_id)
      : target_station_id(target_station_id),
        target_time(target_time),
        is_arrival(is_arrival),
        line_id(std::move(line_id)) {}

  uint64_t target_station_id : 31;
  uint64_t target_time : 16;
  uint64_t is_arrival : 1;
  std::string line_id;

  friend bool operator<(secondary_trip_id const& lhs,
                        secondary_trip_id const& rhs) {
    return std::tie(*reinterpret_cast<uint64_t const*>(&lhs), lhs.line_id) <
           std::tie(*reinterpret_cast<uint64_t const*>(&rhs), rhs.line_id);
  }

  friend bool operator==(secondary_trip_id const& lhs,
                         secondary_trip_id const& rhs) {
    return std::tie(*reinterpret_cast<uint64_t const*>(&lhs), lhs.line_id) ==
           std::tie(*reinterpret_cast<uint64_t const*>(&rhs), rhs.line_id);
  }
};

struct full_trip_id {
  friend bool operator<(full_trip_id const& lhs, full_trip_id const& rhs) {
    return std::tie(lhs.primary, lhs.secondary) <
           std::tie(rhs.primary, rhs.secondary);
  }

  friend bool operator==(full_trip_id const& lhs, full_trip_id const& rhs) {
    return std::tie(lhs.primary, lhs.secondary) ==
           std::tie(rhs.primary, rhs.secondary);
  }

  primary_trip_id primary;
  secondary_trip_id secondary;
};

struct trip {
  trip(full_trip_id id) : id(std::move(id)), edges(nullptr), lcon_idx(0) {}

  full_trip_id id;
  std::vector<edge*> const* edges;
  size_t lcon_idx;
};

using trips = std::vector<trip*>;

}  // namespace motis
