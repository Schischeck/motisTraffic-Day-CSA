#pragma once

#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/event.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

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
// TODO(Mohammad Keyhani)
#if 1
  (void)route_node;
  (void)sched;
  return (type == departure ? lc.d_time_ : lc.a_time_);
#else
  time const event_time = (type == departure ? lc.d_time_ : lc.a_time_);
  auto const it = sched.graph_to_delay_info_.find(graph_event(
      route_node.get_station()->id_, lc.full_con_->con_info_->train_nr_,
      type == departure, event_time, route_node.route_));
  if (it == sched.graph_to_delay_info_.end()) {
    return event_time;
  }
  return it->second->schedule_event_.schedule_time_;
#endif
}

}  // namespace time_util
}  // namespace reliability
}  // namespace motis
