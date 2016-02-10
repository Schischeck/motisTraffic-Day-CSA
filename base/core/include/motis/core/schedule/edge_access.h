#pragma once

#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/edges.h"

namespace motis {

template <typename F>
void foreach_departure_in(edge const& edge, time begin, time end, F fun) {
  if (edge.type() != edge::ROUTE_EDGE) {
    return;
  }

  auto const& conns = edge._m._route_edge._conns;
  auto it = std::lower_bound(std::begin(conns), std::end(conns),
                             light_connection(begin));
  for (; it != std::end(conns) && it->d_time < end; ++it) {
    fun(*it);
  }
}

template <typename F>
void foreach_arrival_in(edge const& edge, time begin, time end, F fun) {
  if (edge.type() != edge::ROUTE_EDGE) {
    return;
  }

  auto const& conns = edge._m._route_edge._conns;
  auto it = std::lower_bound(std::begin(conns), std::end(conns), begin,
                             [](light_connection const& lcon, time const& t) {
                               return lcon.a_time < t;
                             });
  for (; it != std::end(conns) && it->a_time < end; ++it) {
    fun(*it);
  }
}

}  // namespace motis