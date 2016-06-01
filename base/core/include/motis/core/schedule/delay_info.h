#pragma once

#include <ostream>

#include "motis/core/schedule/event.h"
#include "motis/core/schedule/time.h"

namespace motis {

struct delay_info {
  // The order is important:
  // If (forecast time == schedule time), the reason should be FORECAST.
  // Thus, FORECAST needs to appear after SCHEDULE (higher priority last).
  enum class reason { IS, REPAIR, SCHEDULE, PROPAGATION, FORECAST };

  friend std::ostream& operator<<(std::ostream& out,
                                  delay_info::reason const& r) {
    switch (r) {
      case delay_info::reason::SCHEDULE: return out << "SCHEDULE";
      case delay_info::reason::IS: return out << "IS";
      case delay_info::reason::FORECAST: return out << "FORECAST";
      case delay_info::reason::PROPAGATION: return out << "PROPAGATION";
      case delay_info::reason::REPAIR: return out << "REPAIR";
      default: return out;
    }
  }

  delay_info(time schedule_time)
      : repair_time_(0),
        is_time_(0),
        schedule_time_(schedule_time),
        forecase_time_(0),
        propagation_time_(0) {}

  void set(reason const r, time const t) {
    switch (r) {
      case reason::SCHEDULE: schedule_time_ = t; break;
      case reason::IS: is_time_ = t; break;
      case reason::FORECAST: forecase_time_ = t; break;
      case reason::PROPAGATION: propagation_time_ = t; break;
      case reason::REPAIR: repair_time_ = t; break;
    }
  }

  time get_current_time() const {
    switch (get_reason()) {
      case reason::SCHEDULE: return schedule_time_;
      case reason::IS: return is_time_;
      case reason::FORECAST: return forecase_time_;
      case reason::PROPAGATION: return propagation_time_;
      case reason::REPAIR: return repair_time_;
      default: return schedule_time_;
    }
  }

  reason get_reason() const {
    if (repair_time_ != 0) {
      return reason::REPAIR;
    } else if (is_time_ != 0) {
      return reason::IS;
    } else {
      auto const times = {
          std::make_pair(schedule_time_, reason::SCHEDULE),
          std::make_pair(forecase_time_, reason::FORECAST),
          std::make_pair(propagation_time_, reason::PROPAGATION)};
      return std::max_element(begin(times), end(times))->second;
    }
  }

  time repair_time_, is_time_;
  time schedule_time_, forecase_time_, propagation_time_;
};

}  // namespace motis
