#pragma once

#include <cassert>
#include <ostream>
#include <iostream>

#include "motis/core/common/hash_helper.h"
#include "motis/core/schedule/time.h"

namespace motis {

struct base_event {
  base_event(unsigned station_index, uint32_t train_nr, bool departure)
      : _station_index(station_index),
        _train_nr(train_nr),
        _departure(departure) {}

  base_event() : _station_index(0), _train_nr(0), _departure(false) {}

  inline bool departure() const { return _departure; }
  inline bool arrival() const { return !_departure; }
  inline bool found() const { return _station_index != 0; }

  unsigned _station_index;
  uint32_t _train_nr;
  bool _departure;
};

struct graph_event;

struct schedule_event : public base_event {
  schedule_event(unsigned station_index, uint32_t train_nr, bool departure,
                 motis::time schedule_time)
      : base_event(station_index, train_nr, departure),
        _schedule_time(schedule_time) {}

  schedule_event() : base_event(), _schedule_time(INVALID_TIME) {}

  motis::time time() const { return _schedule_time; }

  inline bool valid() const {
    return _schedule_time != INVALID_TIME && _station_index != 0;
  }

  friend bool operator==(schedule_event const& lhs, const schedule_event& rhs) {
    return std::tie(lhs._station_index, lhs._train_nr, lhs._departure,
                    lhs._schedule_time) ==
           std::tie(rhs._station_index, rhs._train_nr, rhs._departure,
                    rhs._schedule_time);
  }

  friend bool operator<(schedule_event const& lhs, const schedule_event& rhs) {
    if (lhs._schedule_time != rhs._schedule_time) {
      return lhs._schedule_time < rhs._schedule_time;
    } else {
      return lhs._station_index == rhs._station_index
                 ? !lhs._departure && rhs._departure
                 : lhs._departure && !rhs._departure;
    }
  }

  friend std::ostream& operator<<(std::ostream& os, const schedule_event& e) {
    os << "<SE st=" << e._station_index << ", tr=" << e._train_nr << ", "
       << (e._departure ? "dep" : "arr")
       << ", t=" << motis::format_time(e._schedule_time) << ">";
    return os;
  }

  motis::time _schedule_time;
};

}  // namespace motis

namespace std {
template <>
struct hash<motis::schedule_event> {
  std::size_t operator()(motis::schedule_event const& e) const {
    std::size_t seed = 0;
    motis::hash_combine(seed, e._station_index);
    motis::hash_combine(seed, e._train_nr);
    motis::hash_combine(seed, e._departure);
    motis::hash_combine(seed, e._schedule_time);
    return seed;
  }
};
}  // namespace std

namespace motis {

struct graph_event : public base_event {
  graph_event(unsigned station_index, uint32_t train_nr, bool departure,
              motis::time current_time, int32_t route_id)
      : base_event(station_index, train_nr, departure),
        _current_time(current_time),
        _route_id(route_id) {}

  explicit graph_event(schedule_event const& se)
      : base_event(se._station_index, se._train_nr, se._departure),
        _current_time(se._schedule_time),
        _route_id(-1) {}

  graph_event() : base_event(), _current_time(INVALID_TIME), _route_id(-1) {}

  motis::time time() const { return _current_time; }

  bool valid() { return _current_time != INVALID_TIME && _station_index != 0; }

  friend bool operator==(graph_event const& lhs, const graph_event& rhs) {
    // assert((lhs._route_id != -1 && rhs._route_id != -1) ||
    //        (lhs._route_id == -1 && rhs._route_id == -1));

    return lhs._station_index == rhs._station_index &&
           lhs._train_nr == rhs._train_nr && lhs._departure == rhs._departure &&
           lhs._current_time == rhs._current_time &&
           lhs._route_id == rhs._route_id;
  }

  friend bool operator<(graph_event const& lhs, const graph_event& rhs) {
    return lhs._current_time == rhs._current_time
               ? lhs._departure && !rhs._departure
               : lhs._current_time < rhs._current_time;
  }

  friend std::ostream& operator<<(std::ostream& os, const graph_event& e) {
    os << "<CE st=" << e._station_index << ", tr=" << e._train_nr << ", "
       << (e._departure ? "dep" : "arr")
       << ", t=" << motis::format_time(e._current_time) << ", r=" << e._route_id
       << ">";
    return os;
  }

  motis::time _current_time;
  int32_t _route_id;
};

}  // namespace motis

namespace std {
template <>
struct hash<motis::graph_event> {
  std::size_t operator()(motis::graph_event const& e) const {
    std::size_t seed = 0;
    motis::hash_combine(seed, e._station_index);
    motis::hash_combine(seed, e._train_nr);
    motis::hash_combine(seed, e._departure);
    motis::hash_combine(seed, e._current_time);
    return seed;
  }
};
}  // namespace std
