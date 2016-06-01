#pragma once

#include <cassert>
#include <iostream>
#include <ostream>
#include <type_traits>

#include "motis/core/common/hash_helper.h"
#include "motis/core/schedule/time.h"
#include "motis/core/schedule/trip.h"

namespace motis {

enum class event_type { DEP, ARR };

struct schedule_event {
  schedule_event() = default;
  schedule_event(primary_trip_id trp, unsigned station_idx, event_type type,
                 time schedule_time)
      : trp_(trp),
        station_idx_(station_idx),
        type_(type),
        schedule_time_(schedule_time) {}

  friend bool operator==(schedule_event const& lhs, const schedule_event& rhs) {
    return std::tie(lhs.trp_, lhs.station_idx_, lhs.type_,
                    lhs.schedule_time_) ==
           std::tie(rhs.trp_, rhs.station_idx_, rhs.type_, rhs.schedule_time_);
  }

  friend bool operator<(schedule_event const& lhs, const schedule_event& rhs) {
    return std::tie(lhs.trp_, lhs.station_idx_, lhs.type_, lhs.schedule_time_) <
           std::tie(rhs.trp_, rhs.station_idx_, rhs.type_, rhs.schedule_time_);
  }

  primary_trip_id trp_;
  unsigned station_idx_;
  event_type type_;
  time schedule_time_;
};

struct graph_event {
  graph_event() = default;
  graph_event(node const* route_node, std::size_t lcon_idx, event_type type)
      : route_node_(route_node), lcon_idx_(lcon_idx), type_(type) {}

  friend bool operator==(graph_event const& lhs, const graph_event& rhs) {
    return std::tie(lhs.route_node_, lhs.lcon_idx_, lhs.type_) ==
           std::tie(rhs.route_node_, rhs.lcon_idx_, rhs.type_);
  }

  friend bool operator<(graph_event const& lhs, const graph_event& rhs) {
    return std::tie(lhs.route_node_, lhs.lcon_idx_, lhs.type_) <
           std::tie(rhs.route_node_, rhs.lcon_idx_, rhs.type_);
  }

  node const* route_node_;
  std::size_t lcon_idx_;
  event_type type_;
};

}  // namespace motis

namespace std {

template <>
struct hash<motis::schedule_event> {
  std::size_t operator()(motis::schedule_event const& e) const {
    std::size_t seed = 0;
    motis::hash_combine(seed, e.trp_);
    motis::hash_combine(seed, e.station_idx_);
    motis::hash_combine(seed, e.type_ == motis::event_type::DEP ? 0 : 1);
    motis::hash_combine(seed, e.schedule_time_);
    return seed;
  }
};

template <>
struct hash<motis::graph_event> {
  std::size_t operator()(motis::graph_event const& e) const {
    std::size_t seed = 0;
    motis::hash_combine(seed, e.route_node_);
    motis::hash_combine(seed, e.lcon_idx_);
    motis::hash_combine(seed, e.type_ == motis::event_type::DEP ? 0 : 1);
    return seed;
  }
};

}  // namespace std
