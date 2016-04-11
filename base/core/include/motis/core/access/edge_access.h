#pragma once

#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/edges.h"

namespace motis {

template <typename F>
void foreach_departure_in(edge const& edge, time begin, time end, F fun) {
  if (edge.type() != edge::ROUTE_EDGE) {
    return;
  }

  auto const& conns = edge.m_.route_edge_.conns_;
  auto it = std::lower_bound(std::begin(conns), std::end(conns),
                             light_connection(begin));
  for (; it != std::end(conns) && it->d_time_ < end; ++it) {
    fun(it);
  }
}

template <typename F>
void foreach_arrival_in(edge const& edge, time begin, time end, F fun) {
  if (edge.type() != edge::ROUTE_EDGE) {
    return;
  }

  auto const& conns = edge.m_.route_edge_.conns_;
  auto it = std::lower_bound(std::begin(conns), std::end(conns), begin,
                             [](light_connection const& lcon, time const& t) {
                               return lcon.a_time_ < t;
                             });
  for (; it != std::end(conns) && it->a_time < end; ++it) {
    fun(it);
  }
}

inline light_connection const& get_lcon(edge const* e, size_t const index) {
  assert(e->type() == edge::ROUTE_EDGE);
  assert(index < e->m_.route_edge_.conns_.size());
  return e->m_.route_edge_.conns_[index];
}

}  // namespace motis
