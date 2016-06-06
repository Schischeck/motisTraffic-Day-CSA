#pragma once

#include "motis/core/schedule/event.h"

namespace motis {

template <typename Fn>
void for_each_departure(ev_key const& arr, Fn fn) {
  for (auto const& e : arr.route_edge_->to_->edges_) {
    if (e.empty()) {
      continue;
    }

    auto dep_lcon = e.get_connection(arr.lcon()->a_time_);
    if (dep_lcon == nullptr) {
      continue;
    }

    fn(ev_key(&e, get_lcon_index(&e, dep_lcon), event_type::DEP));
  }
}

template <typename Fn>
void for_each_arrival(ev_key const& dep, Fn fn) {
  for (auto const& e : dep.route_edge_->from_->incoming_edges_) {
    if (e->empty()) {
      continue;
    }

    auto arr_lcon = e->get_connection_reverse(dep.get_time());
    if (arr_lcon == nullptr) {
      continue;
    }

    fn(ev_key(e, get_lcon_index(e, arr_lcon), event_type::ARR));
  }
}

}  // namespace motis
