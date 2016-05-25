#pragma once

#include "motis/core/schedule/schedule.h"

namespace motis {

time get_schedule_time(schedule const& sched, graph_event const& ev) {
  auto it = sched.graph_to_delay_info_.find(ev);
  if (it == end(sched.graph_to_delay_info_)) {
    return ev.current_time_;
  } else {
    return it->second->schedule_event_.schedule_time_;
  }
}

}  // namespace motis
