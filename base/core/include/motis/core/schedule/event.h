#pragma once

#include <cassert>
#include <iostream>
#include <ostream>

#include "motis/core/common/hash_helper.h"
#include "motis/core/schedule/time.h"

namespace motis {

struct base_event {
  base_event(unsigned station_index, uint32_t train_nr, bool departure)
      : station_index_(station_index),
        train_nr_(train_nr),
        departure_(departure) {}

  base_event() : station_index_(0), train_nr_(0), departure_(false) {}

  inline bool departure() const { return departure_; }
  inline bool arrival() const { return !departure_; }
  inline bool found() const { return station_index_ != 0; }

  unsigned station_index_;
  uint32_t train_nr_;
  bool departure_;
};

struct graph_event;

struct schedule_event : public base_event {
  schedule_event(unsigned station_index, uint32_t train_nr, bool departure,
                 motis::time schedule_time)
      : base_event(station_index, train_nr, departure),
        schedule_time_(schedule_time) {}

  schedule_event() : base_event(), schedule_time_(INVALID_TIME) {}

  motis::time time() const { return schedule_time_; }

  inline bool valid() const {
    return schedule_time_ != INVALID_TIME && station_index_ != 0;
  }

  friend bool operator==(schedule_event const& lhs, const schedule_event& rhs) {
    return std::tie(lhs.station_index_, lhs.train_nr_, lhs.departure_,
                    lhs.schedule_time_) ==
           std::tie(rhs.station_index_, rhs.train_nr_, rhs.departure_,
                    rhs.schedule_time_);
  }

  friend bool operator<(schedule_event const& lhs, const schedule_event& rhs) {
    if (lhs.schedule_time_ != rhs.schedule_time_) {
      return lhs.schedule_time_ < rhs.schedule_time_;
    } else {
      return lhs.station_index_ == rhs.station_index_
                 ? !lhs.departure_ && rhs.departure_
                 : lhs.departure_ && !rhs.departure_;
    }
  }

  friend std::ostream& operator<<(std::ostream& os, const schedule_event& e) {
    os << "<SE st=" << e.station_index_ << ", tr=" << e.train_nr_ << ", "
       << (e.departure_ ? "dep" : "arr")
       << ", t=" << motis::format_time(e.schedule_time_) << ">";
    return os;
  }

  motis::time schedule_time_;
};

}  // namespace motis

namespace std {
template <>
struct hash<motis::schedule_event> {
  std::size_t operator()(motis::schedule_event const& e) const {
    std::size_t seed = 0;
    motis::hash_combine(seed, e.station_index_);
    motis::hash_combine(seed, e.train_nr_);
    motis::hash_combine(seed, e.departure_);
    motis::hash_combine(seed, e.schedule_time_);
    return seed;
  }
};
}  // namespace std

namespace motis {

struct graph_event : public base_event {
  graph_event(unsigned station_index, uint32_t train_nr, bool departure,
              motis::time current_time, int32_t route_id)
      : base_event(station_index, train_nr, departure),
        current_time_(current_time),
        route_id_(route_id) {}

  explicit graph_event(schedule_event const& se)
      : base_event(se.station_index_, se.train_nr_, se.departure_),
        current_time_(se.schedule_time_),
        route_id_(-1) {}

  graph_event() : base_event(), current_time_(INVALID_TIME), route_id_(-1) {}

  motis::time time() const { return current_time_; }

  bool valid() { return current_time_ != INVALID_TIME && station_index_ != 0; }

  friend bool operator==(graph_event const& lhs, const graph_event& rhs) {
    return lhs.station_index_ == rhs.station_index_ &&
           lhs.train_nr_ == rhs.train_nr_ && lhs.departure_ == rhs.departure_ &&
           lhs.current_time_ == rhs.current_time_ &&
           lhs.route_id_ == rhs.route_id_;
  }

  friend bool operator<(graph_event const& lhs, const graph_event& rhs) {
    return lhs.current_time_ == rhs.current_time_
               ? lhs.departure_ && !rhs.departure_
               : lhs.current_time_ < rhs.current_time_;
  }

  friend std::ostream& operator<<(std::ostream& os, const graph_event& e) {
    os << "<CE st=" << e.station_index_ << ", tr=" << e.train_nr_ << ", "
       << (e.departure_ ? "dep" : "arr")
       << ", t=" << motis::format_time(e.current_time_) << ", r=" << e.route_id_
       << ">";
    return os;
  }

  motis::time current_time_;
  int32_t route_id_;
};

}  // namespace motis

namespace std {
template <>
struct hash<motis::graph_event> {
  std::size_t operator()(motis::graph_event const& e) const {
    std::size_t seed = 0;
    motis::hash_combine(seed, e.station_index_);
    motis::hash_combine(seed, e.train_nr_);
    motis::hash_combine(seed, e.departure_);
    motis::hash_combine(seed, e.current_time_);
    return seed;
  }
};
}  // namespace std
