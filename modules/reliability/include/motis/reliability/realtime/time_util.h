#pragma once

#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/event.h"
#include "motis/core/schedule/event_type.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

#include "motis/core/access/realtime_access.h"

#include "motis/reliability/graph_accessor.h"

namespace motis {
namespace reliability {
namespace time_util {

inline motis::time get_scheduled_event_time(node const& route_node,
                                            light_connection const& lc,
                                            event_type type,
                                            schedule const& sched) {
  return get_schedule_time(
      sched, type == event_type::DEP
                 ? graph_accessor::get_departing_route_edge(route_node)
                 : graph_accessor::get_arriving_route_edge(route_node),
      &lc, type);
}

}  // namespace time_util
}  // namespace reliability
}  // namespace motis
