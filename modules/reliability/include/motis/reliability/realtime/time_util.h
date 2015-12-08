#pragma once

#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/event.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

namespace motis {
namespace reliability {
namespace time_util {

enum event_type { departure, arrival };

inline motis::time get_scheduled_event_time(node const& route_node,
                                            light_connection const& lc,
                                            event_type type,
                                            schedule const& sched) {
  time const event_time = (type == departure ? lc.d_time : lc.a_time);
  auto const it = sched.graph_to_delay_info.find(graph_event(
      route_node.get_station()->_id, lc._full_con->con_info->train_nr,
      type == departure, event_time, route_node._route));
  if (it == sched.graph_to_delay_info.end()) {
    return event_time;
  }
  return it->second->_schedule_event._schedule_time;
}

}  // namespace time_util
}  // namespace reliability
}  // namespace motis
