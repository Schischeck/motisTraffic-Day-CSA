#pragma once

#include "motis/core/schedule/event.h"

namespace motis {
namespace rt {

inline bool overtakes(ev_key const& k) {
  auto const get_event_time = [](light_connection const& lcon,
                                 event_type const ev_type) {
    return ev_type == event_type::DEP ? lcon.d_time_ : lcon.a_time_;
  };

  auto const next_lcon = [](array<light_connection> const& conns,
                            light_connection const* it) {
    return ++it == end(conns) ? nullptr : it;
  };

  auto const prev_lcon = [](array<light_connection> const& conns,
                            light_connection const* it) {
    return it == begin(conns) ? nullptr : --it;
  };

  auto const& conns = k.route_edge_->m_.route_edge_.conns_;
  auto const ev_time = get_event_time(*k.lcon(), k.ev_type_);
  auto const next = next_lcon(conns, k.lcon());
  auto const prev = prev_lcon(conns, k.lcon());
  return (next && get_event_time(*next, k.ev_type_) <= ev_time) ||
         (prev && get_event_time(*prev, k.ev_type_) >= ev_time);
}

inline bool conflicts(ev_key const& k) {
  switch (k.ev_type_) {
    case event_type::ARR: {
      auto const dep_time = get_time(k.get_opposite());
      auto const arr_time = get_time(k);
      if (dep_time > arr_time) {
        return true;
      }

      bool valid = true;
      for_each_departure(k, [&](ev_key const& dep) {
        valid = valid && get_time(dep) >= arr_time;
      });
      return !valid;
    }

    case event_type::DEP: {
      auto const dep_time = get_time(k);
      auto const arr_time = get_time(k.get_opposite());
      if (dep_time > arr_time) {
        return true;
      }

      bool valid = true;
      for_each_arrival(k, [&](ev_key const& arr) {
        valid = valid && get_time(arr) <= dep_time;
      });
      return !valid;
    }

    default: return true;
  }
}

}  // namespace rt
}  // namespace motis
