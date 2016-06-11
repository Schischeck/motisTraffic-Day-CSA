#pragma once

#include "motis/core/schedule/event.h"
#include "motis/core/access/time_access.h"

namespace motis {

template <typename Fn>
void for_each_departure(ev_key const& arr, Fn fn) {
  for (auto const& e : arr.route_edge_->to_->edges_) {
    if (!e.empty()) {
      fn(ev_key(&e, arr.lcon_idx_, event_type::DEP));
    }
  }
}

template <typename Fn>
void for_each_arrival(ev_key const& dep, Fn fn) {
  for (auto const& e : dep.route_edge_->from_->incoming_edges_) {
    if (!e->empty()) {
      fn(ev_key(e, dep.lcon_idx_, event_type::ARR));
    }
  }
}

inline time get_time(ev_key const& k) {
  return get_time(&get_lcon(k.route_edge_, k.lcon_idx_), k.ev_type_);
}

}  // namespace motis
