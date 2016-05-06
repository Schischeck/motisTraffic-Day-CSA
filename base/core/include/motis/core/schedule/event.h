#pragma once

#include <cassert>
#include <iostream>
#include <ostream>

#include "motis/core/common/hash_helper.h"
#include "motis/core/schedule/time.h"

namespace motis {

struct base_event {
  enum class event_type { DEP, ARR };

  trip const* trp_;
  unsigned station_idx_;
  event_type type_;
};

struct schedule_event : public base_event {
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
    motis::hash_combine(seed, e.type_);
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
    motis::hash_combine(seed, e.type_);
    motis::hash_combine(seed, e.current_time_);
    return seed;
  }
};

}  // namespace std
