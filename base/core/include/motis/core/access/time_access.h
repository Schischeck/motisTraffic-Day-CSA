#pragma once

#include "motis/core/common/date_time_util.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

namespace motis {

constexpr auto kDefaultTimezoneOffset = 60;

inline std::time_t motis_to_unixtime(schedule const& sched, time t) {
  return motis_to_unixtime(sched.schedule_begin_, t);
}

inline time unix_to_motistime(schedule const& sched, std::time_t t) {
  return unix_to_motistime(sched.schedule_begin_, t);
}

inline std::time_t external_schedule_begin(schedule const& sched) {
  return sched.schedule_begin_ + SCHEDULE_OFFSET_MINUTES * 60;
}

inline std::time_t external_schedule_end(schedule const& sched) {
  return sched.schedule_end_;
}

inline time motis_time(int const hhmm, int const day_idx = 0,
                       int const timezone_offset = kDefaultTimezoneOffset) {
  return SCHEDULE_OFFSET_MINUTES + day_idx * MINUTES_A_DAY + hhmm_to_min(hhmm) -
         timezone_offset;
}

inline std::time_t unix_time(
    schedule const& sched, int const hhmm, int const day_idx = 0,
    int const timezone_offset = kDefaultTimezoneOffset) {
  return motis_to_unixtime(sched, motis_time(hhmm, day_idx, timezone_offset));
}

}  // namespace motis
