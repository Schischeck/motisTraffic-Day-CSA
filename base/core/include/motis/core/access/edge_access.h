#pragma once

#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/edges.h"

namespace motis {

inline edge const* get_route_edge(node const* route_node,
                                  light_connection const* lcon,
                                  event_type const ev_type) {
  if (ev_type == event_type::DEP) {
    for (auto const& e : route_node->edges_) {
      if (e.type() == edge::ROUTE_EDGE &&
          e.m_.route_edge_.conns_.contains(lcon)) {
        return &e;
      }
    }
  } else {
    for (auto const& e : route_node->incoming_edges_) {
      if (e->type() == edge::ROUTE_EDGE &&
          e->m_.route_edge_.conns_.contains(lcon)) {
        return e;
      }
    }
  }
  throw std::runtime_error("get_route_edge(): light connection not found");
}

inline node* get_route_node(edge const& e, event_type const ev_type) {
  return ev_type == event_type::DEP ? e.from_ : e.to_;
}

inline light_connection const& get_lcon(edge const* route_edge,
                                        size_t const index) {
  assert(route_edge->type() == edge::ROUTE_EDGE);
  assert(index < route_edge->m_.route_edge_.conns_.size());
  return route_edge->m_.route_edge_.conns_[index];
}

inline time get_time(light_connection const* lcon, event_type const ev_type,
                     int day) {
  return lcon->event_time(ev_type, day);
}

inline time get_time(edge const* route_edge, std::size_t const lcon_index,
                     event_type const ev_type, int day) {
  return get_time(&get_lcon(route_edge, lcon_index), ev_type, day);
}

inline std::size_t get_lcon_index(edge const* route_edge,
                                  light_connection const* lcon) {
  auto const& lcons = route_edge->m_.route_edge_.conns_;
  if (lcon < begin(lcons) || lcon >= end(lcons)) {
    throw std::runtime_error("get_lcon_index(): light connection not found");
  }
  return std::distance(begin(lcons), lcon);
}

}  // namespace motis
