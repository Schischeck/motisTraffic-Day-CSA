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
  explicit delay_info(struct schedule_event schedule_event,
                      int32_t route_id = -1)
      : schedule_event_(schedule_event),
        route_id_(route_id),
        forecast_time_(motis::INVALID_TIME),
        current_time_(schedule_event.schedule_time_),
        canceled_(false),
        reason_(timestamp_reason::SCHEDULE) {}

  inline bool delayed() const {
    return current_time_ != schedule_event_.schedule_time_;
  }

  inline bool canceled() const { return canceled_; }

  inline schedule_event sched_ev() const { return schedule_event_; }

  inline graph_event graph_ev() const {
    return graph_event(schedule_event_.station_index_,
                       schedule_event_.train_nr_, schedule_event_.departure_,
                       current_time_, route_id_);
  }

  bool operator==(const delay_info& other) const {
    return schedule_event_ == other.schedule_event_;
  }

  friend std::ostream& operator<<(std::ostream& os, const delay_info& di) {
    os << "<event=" << di.schedule_event_ << ", route_id=" << di.route_id_
       << ", fc=" << motis::format_time(di.forecast_time_)
       << ", ct=" << motis::format_time(di.current_time_)
       << ", reason=" << di.reason_ << (di.canceled() ? ", CANCELED" : "")
       << ">";
    return os;
  }

  struct schedule_event schedule_event_;
  int32_t route_id_;

  motis::time forecast_time_;
  motis::time current_time_;
  bool canceled_;
  timestamp_reason reason_;
};

}  // namespace motis
