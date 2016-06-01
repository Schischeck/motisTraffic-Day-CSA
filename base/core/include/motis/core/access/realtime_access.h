#pragma once

#include <cassert>

#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"
#include "motis/core/access/edge_access.h"

namespace motis {

inline time get_schedule_time(schedule const& sched, node const* route_node,
                              std::size_t const lcon_index,
                              event_type const ev_type) {
  auto it = sched.graph_to_delay_info_.find({route_node, lcon_index, ev_type});
  if (it == end(sched.graph_to_delay_info_)) {
    return get_time(route_node, lcon_index, ev_type);
  } else {
    return it->second->schedule_time_;
  }
}

inline time get_schedule_time(schedule const& sched, node const* route_node,
                              light_connection const* lcon,
                              event_type const ev_type) {
  auto it = sched.graph_to_delay_info_.find(
      {route_node, get_lcon_index(route_node, lcon, ev_type), ev_type});

  printf("#entries=%d\n", sched.graph_to_delay_info_.size());
  for (auto const& e : sched.graph_to_delay_info_) {
    printf("  entry: %p\n", e.second);
  }

  if (it == end(sched.graph_to_delay_info_)) {
    return ev_type == event_type::DEP ? lcon->d_time_ : lcon->a_time_;
  } else {
    printf("\n\nFOUND:\n");
    printf("  schedule: %d\n", motis_to_unixtime(sched, it->second->schedule_time_));
    printf("   current: %d\n", motis_to_unixtime(sched, it->second->get_current_time()));
    printf("\n");
    return it->second->schedule_time_;
  }
}

}  // namespace motis
