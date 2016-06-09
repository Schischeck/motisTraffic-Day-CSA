#pragma once

#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/event.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

#include "motis/core/access/realtime_access.h"

#include "motis/reliability/graph_accessor.h"

namespace motis {
namespace reliability {
namespace time_util {

enum event_type { departure, arrival };

// TODO(Mohammad Keyhani)
// parts of data_departure::init_departure_time,
// data_arrival::init_arrival_time,
// and interchange_info (connection_graph_rating)
// have to be moved to this file

inline motis::time get_scheduled_event_time(node const& route_node,
                                            light_connection const& lc,
                                            event_type type,
                                            schedule const& sched) {
  return get_schedule_time(
      sched,
      type == departure ? graph_accessor::get_departing_route_edge(route_node)
                        : graph_accessor::get_arriving_route_edge(route_node),
      &lc, type == departure ? motis::event_type::DEP : motis::event_type::DEP);
}

}  // namespace time_util
}  // namespace reliability
}  // namespace motis
