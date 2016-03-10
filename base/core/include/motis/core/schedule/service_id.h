#pragma once

#include <cinttypes>

#include <vector>
#include <utility>

#include "motis/core/schedule/edges.h"

namespace motis {

struct primary_service_id {
  primary_service_id() = default;
  primary_service_id(uint32_t station_id, motis::time time, uint32_t train_nr)
      : station_id(station_id), train_nr(train_nr), time(time) {}

  uint64_t station_id : 31;
  uint64_t train_nr : 17;
  uint64_t time : 16;

  friend bool operator<(primary_service_id const& lhs,
                        primary_service_id const& rhs) {
    return *reinterpret_cast<uint64_t const*>(&lhs) <
           *reinterpret_cast<uint64_t const*>(&rhs);
  }

  friend bool operator==(primary_service_id const& lhs,
                         primary_service_id const& rhs) {
    return *reinterpret_cast<uint64_t const*>(&lhs) ==
           *reinterpret_cast<uint64_t const*>(&rhs);
  }
};

}  // namespace motis

namespace std {
template <>
struct hash<motis::primary_service_id> {
  std::size_t operator()(motis::primary_service_id const& e) const {
    return *reinterpret_cast<uint64_t const*>(&e);
  }
};
}  // namespace std

namespace motis {

struct secondary_service_id {
  secondary_service_id() = default;
  secondary_service_id(uint32_t line_id, uint32_t target_station_id,
                       uint16_t target_time, uint32_t is_arrival)
      : line_id(line_id),
        target_station_id(target_station_id),
        target_time(target_time),
        is_arrival(is_arrival) {}

  uint64_t line_id : 25;
  uint64_t target_station_id : 22;
  uint64_t target_time : 16;
  uint64_t is_arrival : 1;

  friend bool operator<(secondary_service_id const& lhs,
                        secondary_service_id const& rhs) {
    return *reinterpret_cast<uint64_t const*>(&lhs) <
           *reinterpret_cast<uint64_t const*>(&rhs);
  }

  friend bool operator==(secondary_service_id const& lhs,
                         secondary_service_id const& rhs) {
    return *reinterpret_cast<uint64_t const*>(&lhs) ==
           *reinterpret_cast<uint64_t const*>(&rhs);
  }
};

struct full_service_id {
  primary_service_id primary;
  secondary_service_id secondary;
};

struct service {
  full_service_id id;
  edge* first_route_edge;
  size_t light_connection_index;

  service const* next;
};

}  // namespace motis
