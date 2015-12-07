#pragma once

#include "motis/core/schedule/event.h"
#include "motis/core/schedule/time.h"

namespace motis {

enum class timestamp_reason : uint8_t {
  SCHEDULE,
  IS,
  FORECAST,
  PROPAGATION,
  REPAIR
};

inline std::ostream& operator<<(std::ostream& os, const timestamp_reason& r) {
  switch (r) {
    case timestamp_reason::SCHEDULE: os << "s"; break;
    case timestamp_reason::IS: os << "i"; break;
    case timestamp_reason::FORECAST: os << "f"; break;
    case timestamp_reason::PROPAGATION: os << "p"; break;
    case timestamp_reason::REPAIR: os << "r"; break;
  }
  return os;
}

struct delay_info {
  delay_info(class schedule_event schedule_event, int32_t route_id = -1)
      : _schedule_event(schedule_event),
        _route_id(route_id),
        _forecast_time(motis::INVALID_TIME),
        _current_time(schedule_event._schedule_time),
        _canceled(false),
        _reason(timestamp_reason::SCHEDULE) {}

  inline bool delayed() const {
    return _current_time != _schedule_event._schedule_time;
  }
  inline bool canceled() const { return _canceled; }

  inline schedule_event sched_ev() const { return _schedule_event; }

  inline graph_event graph_ev() const {
    return graph_event(_schedule_event._station_index,
                       _schedule_event._train_nr, _schedule_event._departure,
                       _current_time, _route_id);
  }

  bool operator==(const delay_info& other) const {
    return _schedule_event == other._schedule_event;
  }

  friend std::ostream& operator<<(std::ostream& os, const delay_info& di) {
    os << "<event=" << di._schedule_event << ", route_id=" << di._route_id
       << ", fc=" << motis::format_time(di._forecast_time)
       << ", ct=" << motis::format_time(di._current_time)
       << ", reason=" << di._reason << (di.canceled() ? ", CANCELED" : "")
       << ">";
    return os;
  }

  class schedule_event _schedule_event;
  int32_t _route_id;

  motis::time _forecast_time;
  motis::time _current_time;
  bool _canceled;
  timestamp_reason _reason;
};

}  // namespace motis
