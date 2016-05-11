#pragma once

#include <cassert>
#include <iostream>
#include <ostream>
#include <type_traits>

#include "motis/core/common/hash_helper.h"
#include "motis/core/schedule/time.h"
#include "motis/core/schedule/trip.h"

namespace motis {

struct base_event {
  enum class event_type { DEP, ARR };

  base_event() = default;
  base_event(primary_trip_id trp, unsigned station_idx, event_type type)
      : trp_(trp), station_idx_(station_idx), type_(type) {}

  primary_trip_id trp_;
  unsigned station_idx_;
  event_type type_;
};

struct schedule_event : public base_event {
  schedule_event() = default;
  schedule_event(primary_trip_id trp, unsigned station_idx, event_type type,
                 time schedule_time)
      : base_event(trp, station_idx, type), schedule_time_(schedule_time) {}

  friend bool operator==(schedule_event const& lhs, const schedule_event& rhs) {
    return std::tie(lhs.trp_, lhs.station_idx_, lhs.type_,
                    lhs.schedule_time_) ==
           std::tie(rhs.trp_, rhs.station_idx_, rhs.type_, rhs.schedule_time_);
  }

  friend bool operator<(schedule_event const& lhs, const schedule_event& rhs) {
    return std::tie(lhs.trp_, lhs.station_idx_, lhs.type_, lhs.schedule_time_) <
           std::tie(rhs.trp_, rhs.station_idx_, rhs.type_, rhs.schedule_time_);
  }

  time schedule_time_;
};

struct graph_event : public base_event {
  graph_event() = default;
  graph_event(primary_trip_id trp, unsigned station_idx, event_type type,
              time current_time)
      : base_event(trp, station_idx, type), current_time_(current_time) {}

  friend bool operator==(graph_event const& lhs, const graph_event& rhs) {
    return std::tie(lhs.trp_, lhs.station_idx_, lhs.type_, lhs.current_time_) ==
           std::tie(rhs.trp_, rhs.station_idx_, rhs.type_, rhs.current_time_);
  }

  friend bool operator<(graph_event const& lhs, const graph_event& rhs) {
    return std::tie(lhs.trp_, lhs.station_idx_, lhs.type_, lhs.current_time_) <
           std::tie(rhs.trp_, rhs.station_idx_, rhs.type_, rhs.current_time_);
  }

  time current_time_;
};

}  // namespace motis

namespace std {

template <>
struct hash<motis::schedule_event> {
  std::size_t operator()(motis::schedule_event const& e) const {
    std::size_t seed = 0;
    motis::hash_combine(seed, e.trp_);
    motis::hash_combine(seed, e.station_idx_);
    motis::hash_combine(
        seed,
        static_cast<
            typename std::underlying_type<motis::base_event::event_type>::type>(
            e.type_));
    motis::hash_combine(seed, e.schedule_time_);
    return seed;
  }
};

template <>
struct hash<motis::graph_event> {
  std::size_t operator()(motis::graph_event const& e) const {
    std::size_t seed = 0;
    motis::hash_combine(seed, e.trp_);
    motis::hash_combine(seed, e.station_idx_);
    motis::hash_combine(
        seed,
        static_cast<
            typename std::underlying_type<motis::base_event::event_type>::type>(
            e.type_));
    motis::hash_combine(seed, e.current_time_);
    return seed;
  }
};

}  // namespace std
