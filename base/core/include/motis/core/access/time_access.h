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

}  // namespace motis
