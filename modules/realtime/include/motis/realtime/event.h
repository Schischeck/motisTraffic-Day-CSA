#pragma once

#include <ostream>

#include "boost/functional/hash.hpp"
#include "boost/operators.hpp"

#include "motis/core/schedule/time.h"

namespace motis {
namespace realtime {

class base_event {
public:
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

class graph_event;

class schedule_event : public base_event,
                       private boost::less_than_comparable<schedule_event>,
                       private boost::equality_comparable<schedule_event> {
public:
  schedule_event(unsigned station_index, uint32_t train_nr, bool departure,
                 motis::time schedule_time)
      : base_event(station_index, train_nr, departure),
        _schedule_time(schedule_time) {}

  schedule_event() : base_event(), _schedule_time(motis::INVALID_TIME) {}

  motis::time time() const { return _schedule_time; }

  inline bool valid() const {
    return _schedule_time != motis::INVALID_TIME && _station_index != 0;
  }

  bool operator==(const schedule_event& other) const {
    return _station_index == other._station_index &&
           _train_nr == other._train_nr && _departure == other._departure &&
           _schedule_time == other._schedule_time;
  }

  bool operator<(const schedule_event& rhs) const {
    if (_schedule_time != rhs._schedule_time) {
      return _schedule_time < rhs._schedule_time;
    } else {
      return _station_index == rhs._station_index
                 ? !_departure && rhs._departure
                 : _departure && !rhs._departure;
    }
  }

  friend std::size_t hash_value(const schedule_event& e) {
    std::size_t seed = 0;
    boost::hash_combine(seed, e._station_index);
    boost::hash_combine(seed, e._train_nr);
    boost::hash_combine(seed, e._departure);
    boost::hash_combine(seed, e._schedule_time);
    return seed;
  }

  friend std::ostream& operator<<(std::ostream& os, const schedule_event& e) {
    os << "<SE st=" << e._station_index << ", tr=" << e._train_nr << ", "
       << (e._departure ? "dep" : "arr")
       << ", t=" << motis::format_time(e._schedule_time) << ">";
    return os;
  }

  motis::time _schedule_time;
};

class graph_event : public base_event,
                    private boost::less_than_comparable<graph_event>,
                    private boost::equality_comparable<graph_event> {
public:
  graph_event(unsigned station_index, uint32_t train_nr, bool departure,
              motis::time current_time, int32_t route_id)
      : base_event(station_index, train_nr, departure),
        _current_time(current_time),
        _route_id(route_id) {}

  explicit graph_event(const schedule_event& se)
      : base_event(se._station_index, se._train_nr, se._departure),
        _current_time(se._schedule_time),
        _route_id(-1) {}

  graph_event()
      : base_event(), _current_time(motis::INVALID_TIME), _route_id(-1) {}

  motis::time time() const { return _current_time; }

  bool valid() {
    return _current_time != motis::INVALID_TIME && _station_index != 0;
  }

  bool operator==(const graph_event& other) const {
    assert(_route_id != -1 && other._route_id != -1);
    assert(_route_id == -1 && other._route_id == -1);
    return _station_index == other._station_index &&
           _train_nr == other._train_nr && _departure == other._departure &&
           _current_time == other._current_time && _route_id == other._route_id;
  }

  bool operator<(const graph_event& rhs) const {
    return _current_time == rhs._current_time
               ? _departure && !rhs._departure
               : _current_time < rhs._current_time;
  }

  friend std::size_t hash_value(const graph_event& e) {
    std::size_t seed = 0;
    boost::hash_combine(seed, e._station_index);
    boost::hash_combine(seed, e._train_nr);
    boost::hash_combine(seed, e._departure);
    boost::hash_combine(seed, e._current_time);
    return seed;
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

}  // namespace realtime
}  // namespace motis
