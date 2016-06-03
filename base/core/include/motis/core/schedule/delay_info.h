#pragma once

#include <ostream>

#include "motis/core/schedule/event.h"
#include "motis/core/schedule/time.h"

namespace motis {

struct delay_info {
  // The order is important:
  // If (forecast time == schedule time), the reason should be FORECAST.
  // Thus, FORECAST needs to appear after SCHEDULE (higher priority last).
  enum class reason { IS, SCHEDULE, PROPAGATION, FORECAST };

  friend std::ostream& operator<<(std::ostream& out,
                                  delay_info::reason const& r) {
    switch (r) {
      case delay_info::reason::SCHEDULE: return out << "SCHEDULE";
      case delay_info::reason::IS: return out << "IS";
      case delay_info::reason::FORECAST: return out << "FORECAST";
      case delay_info::reason::PROPAGATION: return out << "PROPAGATION";
      default: return out;
    }
  }

  delay_info(delay_info const&) = default;

  delay_info(graph_event ev, time schedule_time)
      : ev_(std::move(ev)),
        is_time_(0),
        schedule_time_(schedule_time),
        forecase_time_(0),
        propagation_time_(0) {}

  time get_schedule_time() const { return schedule_time_; }

  time get_is_time() const { return is_time_; }
  time get_forecast_time() const { return forecase_time_; }
  time get_propagation_time() const { return propagation_time_; }

  void update(delay_info const& d) {
    if (schedule_time_ == 0) {
      *this = d;
    } else {
      if (d.forecase_time_ != 0) {
        forecase_time_ = d.forecase_time_;
      }
      if (d.propagation_time_ != 0) {
        propagation_time_ = d.propagation_time_;
      }
      if (is_time_ != 0) {
        is_time_ = d.is_time_;
      }
    }
  }

  void set(reason const r, time const t) {
    switch (r) {
      case reason::SCHEDULE: schedule_time_ = t; break;
      case reason::IS: is_time_ = t; break;
      case reason::FORECAST: forecase_time_ = t; break;
      case reason::PROPAGATION: propagation_time_ = t; break;
    }
  }

  time get_current_time() const {
    switch (get_reason()) {
      case reason::SCHEDULE: return schedule_time_;
      case reason::IS: return is_time_;
      case reason::FORECAST: return forecase_time_;
      case reason::PROPAGATION: return propagation_time_;
      default: return schedule_time_;
    }
  }

  reason get_reason() const {
    if (is_time_ != 0) {
      return reason::IS;
    } else {
      auto const times = {
          std::make_pair(schedule_time_, reason::SCHEDULE),
          std::make_pair(forecase_time_, reason::FORECAST),
          std::make_pair(propagation_time_, reason::PROPAGATION)};
      return std::max_element(begin(times), end(times))->second;
    }
  }

  graph_event get_graph_event() const { return ev_; }

private:
  graph_event ev_;
  time is_time_;
  time schedule_time_, forecase_time_, propagation_time_;
};

}  // namespace motis
