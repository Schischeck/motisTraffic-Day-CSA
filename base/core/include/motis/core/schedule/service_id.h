#pragma once

#include <cinttypes>

#include <vector>
#include <utility>

#include "motis/common/schedule/edges.h"

namespace motis {

struct service_id_maj {
  service_id_maj() = default;
  service_id_maj(uint32_t station_id, motis::time time, uint32_t train_nr)
      : station_id(station_id), time(time), train_nr(train_nr) {}

  uint64_t station_id : 31;
  uint64_t time : 16;
  uint64_t train_nr : 17;

  friend bool operator<(service_id_maj const& lhs, service_id_maj const& rhs) {
    return *reinterpret_cast<uint64_t const*>(&lhs) <
           *reinterpret_cast<uint64_t const*>(&rhs);
  }

  friend bool operator==(service_id_maj const& lhs, service_id_maj const& rhs) {
    return *reinterpret_cast<uint64_t const*>(&lhs) ==
           *reinterpret_cast<uint64_t const*>(&rhs);
  }
};

}  // namespace motis

namespace std {
template <>
struct hash<motis::service_id_maj> {
  std::size_t operator()(motis::service_id_maj const& e) const {
    return *reinterpret_cast<uint64_t const*>(&e);
  }
};
}  // namespace std

namespace motis {

struct service_id_min {
  service_id_min() = default;
  service_id_min(uint32_t line_id, uint32_t target_station_id,
                 uint16_t target_time, uint32_t event_type)
      : line_id(line_id),
        target_station_id(target_station_id),
        target_time(target_time),
        event_type(event_type) {}

  uint64_t line_id : 25;
  uint64_t target_station_id : 22;
  uint64_t target_time : 16;
  uint64_t event_type : 1;

  friend bool operator<(service_id_min const& lhs, service_id_min const& rhs) {
    return *reinterpret_cast<uint64_t const*>(&lhs) <
           *reinterpret_cast<uint64_t const*>(&rhs);
  }

  friend bool operator==(service_id_min const& lhs, service_id_min const& rhs) {
    return *reinterpret_cast<uint64_t const*>(&lhs) ==
           *reinterpret_cast<uint64_t const*>(&rhs);
  }
};

struct service {
  service_id_maj maj;
  service_id_min min;

  edge* edge;
  size_t light_connection_index;
};

using service_container = std::vector<std::pair<service_id_min, service*>>;

}  // namespace motis
