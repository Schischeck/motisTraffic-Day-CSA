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
  for (; it != std::end(conns) && it->a_time_ < end; ++it) {
    fun(it);
  }
}

inline light_connection const& get_lcon(edge const* e, size_t const index) {
  assert(e->type() == edge::ROUTE_EDGE);
  assert(index < e->m_.route_edge_.conns_.size());
  return e->m_.route_edge_.conns_[index];
}

inline time get_dep_time(node const* route_node, std::size_t const lcon_index) {
  for (auto const& e : route_node->edges_) {
    if (e.type() != edge::ROUTE_EDGE) {
      continue;
    }
    return get_lcon(&e, lcon_index).d_time_;
  }
  throw std::runtime_error("departure light connection not found");
}

inline time get_arr_time(node const* route_node, std::size_t const lcon_index) {
  for (auto const& e : route_node->incoming_edges_) {
    if (e->type() != edge::ROUTE_EDGE) {
      continue;
    }
    return get_lcon(e, lcon_index).a_time_;
  }
  throw std::runtime_error("arrival light connection not found");
}

inline time get_time(node const* route_node, std::size_t const lcon_index,
                     event_type const ev_type) {
  if (ev_type == event_type::DEP) {
    return get_dep_time(route_node, lcon_index);
  } else {
    return get_arr_time(route_node, lcon_index);
  }
}

inline std::size_t get_dep_lcon_index(node const* route_node,
                                      light_connection const* lcon) {
  for (auto const& e : route_node->edges_) {
    if (e.type() != edge::ROUTE_EDGE) {
      continue;
    }

    std::size_t index = 0;
    for (auto const& lcon1 : e.m_.route_edge_.conns_) {
      if (&lcon1 == lcon) {
        return index;
      }
      ++index;
    }
  }
  throw std::runtime_error("departure light connection not found");
}

inline std::size_t get_arr_lcon_index(node const* route_node,
                                      light_connection const* lcon) {
  for (auto const& e : route_node->incoming_edges_) {
    if (e->type() != edge::ROUTE_EDGE) {
      continue;
    }

    std::size_t index = 0;
    for (auto const& lcon1 : e->m_.route_edge_.conns_) {
      if (&lcon1 == lcon) {
        return index;
      }
      ++index;
    }
  }
  throw std::runtime_error("arrival light connection not found");
}

inline std::size_t get_lcon_index(node const* route_node,
                                  light_connection const* lcon,
                                  event_type const type) {
  if (type == event_type::DEP) {
    return get_dep_lcon_index(route_node, lcon);
  } else {
    return get_arr_lcon_index(route_node, lcon);
  }
}

}  // namespace motis
