#pragma once

#include <cassert>
#include <iostream>
#include <ostream>
#include <type_traits>

#include "motis/core/common/hash_helper.h"
#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/time.h"
#include "motis/core/schedule/trip.h"

namespace motis {

class node;
enum class event_type : uint8_t { DEP, ARR };

struct schedule_event {
  schedule_event() = default;
  schedule_event(primary_trip_id trp, unsigned station_idx, event_type type,
                 time schedule_time)
      : trp_(std::move(trp)),
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

struct ev_key {
  ev_key() = default;
  ev_key(edge const* route_edge, std::size_t lcon_idx, event_type type)
      : route_edge_(route_edge), lcon_idx_(lcon_idx), ev_type_(type) {}

  friend bool operator==(ev_key const& lhs, const ev_key& rhs) {
    return std::tie(lhs.route_edge_, lhs.lcon_idx_, lhs.ev_type_) ==
           std::tie(rhs.route_edge_, rhs.lcon_idx_, rhs.ev_type_);
  }

  friend bool operator<(ev_key const& lhs, const ev_key& rhs) {
    return std::tie(lhs.route_edge_, lhs.lcon_idx_, lhs.ev_type_) <
           std::tie(rhs.route_edge_, rhs.lcon_idx_, rhs.ev_type_);
  }

  bool is_arrival() const { return ev_type_ == event_type::ARR; }
  bool is_departure() const { return ev_type_ == event_type::DEP; }

  ev_key get_opposite() const {
    auto const ev_type =
        ev_type_ == event_type::ARR ? event_type::DEP : event_type::ARR;
    return {route_edge_, lcon_idx_, ev_type};
  }

  ev_key get_earlier() const { return {route_edge_, lcon_idx_ - 1, ev_type_}; }
  ev_key get_later() const { return {route_edge_, lcon_idx_ + 1, ev_type_}; }

  light_connection const* lcon() const {
    return &route_edge_->m_.route_edge_.conns_[lcon_idx_];
  }

  time get_time() const {
    return ev_type_ == event_type::DEP ? lcon()->d_time_ : lcon()->a_time_;
  }

  uint32_t get_station_idx() const {
    return (ev_type_ == event_type::DEP ? route_edge_->from_ : route_edge_->to_)
        ->get_station()
        ->id_;
  }

  edge const* route_edge_;
  std::size_t lcon_idx_;
  event_type ev_type_;
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
struct hash<motis::ev_key> {
  std::size_t operator()(motis::ev_key const& e) const {
    std::size_t seed = 0;
    motis::hash_combine(seed, e.route_edge_);
    motis::hash_combine(seed, e.lcon_idx_);
    motis::hash_combine(seed, e.ev_type_ == motis::event_type::DEP ? 0 : 1);
    return seed;
  }
};

}  // namespace std
