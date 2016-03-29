#pragma once

#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

namespace motis {

inline std::time_t motis_to_unixtime(schedule const& sched, time t) {
  return motis::motis_to_unixtime(sched.schedule_begin_, t);
}

inline time unix_to_motistime(schedule const& sched, std::time_t t) {
  return motis::unix_to_motistime(sched.schedule_begin_, t);
}

inline std::time_t external_schedule_begin(schedule const& sched) {
  return sched.schedule_begin_ + SCHEDULE_OFFSET_MINUTES * 60;
}

inline std::time_t external_schedule_end(schedule const& sched) {
  return sched.schedule_end_;
}

}  // namespace motis
