#pragma once

#include <cassert>

#include "motis/core/schedule/event.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"
#include "motis/core/access/edge_access.h"

namespace motis {

inline time get_schedule_time(schedule const& sched, ev_key const& k) {
  auto it = sched.graph_to_delay_info_.find(k);
  if (it == end(sched.graph_to_delay_info_)) {
    return get_time(k.route_edge_, k.lcon_idx_, k.ev_type_);
  } else {
    return it->second->get_schedule_time();
  }
}

inline time get_schedule_time(schedule const& sched, edge const* route_edge,
                              std::size_t const lcon_index,
                              event_type const ev_type) {
  auto it = sched.graph_to_delay_info_.find({route_edge, lcon_index, ev_type});
  if (it == end(sched.graph_to_delay_info_)) {
    return get_time(route_edge, lcon_index, ev_type);
  } else {
    return it->second->get_schedule_time();
  }
}

inline time get_schedule_time(schedule const& sched, edge const* route_edge,
                              light_connection const* lcon,
                              event_type const ev_type) {
  auto it = sched.graph_to_delay_info_.find(
      {route_edge, get_lcon_index(route_edge, lcon), ev_type});
  if (it == end(sched.graph_to_delay_info_)) {
    return ev_type == event_type::DEP ? lcon->d_time_ : lcon->a_time_;
  } else {
    return it->second->get_schedule_time();
  }
}

inline time get_delay(schedule const& sched, ev_key const& k) {
  return get_time(k.lcon(), k.ev_type_) - get_schedule_time(sched, k);
}

inline delay_info get_delay_info(schedule const& sched, node const* route_node,
                                 light_connection const* lcon,
                                 event_type const ev_type) {
  auto route_edge = get_route_edge(route_node, lcon, ev_type);
  auto lcon_idx = get_lcon_index(route_edge, lcon);
  auto it = sched.graph_to_delay_info_.find({route_edge, lcon_idx, ev_type});
  if (it == end(sched.graph_to_delay_info_)) {
    return delay_info{ev_key(route_edge, lcon_idx, ev_type)};
  } else {
    return *it->second;
  }
}

inline delay_info get_delay_info(schedule const& sched, edge const* route_edge,
                                 light_connection const* lcon,
                                 event_type const ev_type) {
  auto lcon_idx = get_lcon_index(route_edge, lcon);
  auto it = sched.graph_to_delay_info_.find({route_edge, lcon_idx, ev_type});
  if (it == end(sched.graph_to_delay_info_)) {
    return delay_info{ev_key(route_edge, lcon_idx, ev_type)};
  } else {
    return *it->second;
  }
}

inline delay_info* get_delay_info(schedule const& sched, ev_key const& k) {
  auto it = sched.graph_to_delay_info_.find(k);
  if (it == end(sched.graph_to_delay_info_)) {
    return nullptr;
  } else {
    return it->second;
  }
}

}  // namespace motis
