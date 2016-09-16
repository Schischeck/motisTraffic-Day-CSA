#pragma once

#include <map>
#include <set>
#include <vector>

#include "motis/core/schedule/nodes.h"

namespace motis {
namespace rt {

inline bool station_contains_node(station_node const* s, node const* n) {
  if (s == n) {
    return true;
  }
  for (auto const& e : s->edges_) {
    if (n == e.to_) {
      return true;
    }
  }
  return false;
}

inline std::map<node const*, std::vector<edge*>> incoming_non_station_edges(
    std::set<station_node*> const& station_nodes) {
  std::map<node const*, std::vector<edge*>> incoming;

  auto const add_incoming = [&](station_node const* s, node const* n) {
    for (auto const& e_in : n->incoming_edges_) {
      if (!station_contains_node(s, e_in->from_)) {
        incoming[n].push_back(e_in);
      }
    }
  };

  for (auto const& s : station_nodes) {
    add_incoming(s, s);
    for (auto const& e : s->edges_) {
      add_incoming(s, e.to_);
    }
  }

  return incoming;
}

inline void add_incoming_station_edges(
    std::set<station_node*> const& station_nodes,
    std::map<node const*, std::vector<edge*>>& incoming) {
  for (auto& s : station_nodes) {
    for (auto& station_edge : s->edges_) {
      incoming[station_edge.to_].push_back(&station_edge);
      for (auto& edge : station_edge.to_->edges_) {
        if (station_contains_node(s, edge.to_)) {
          incoming[edge.to_].push_back(&edge);
        }
      }
    }
  }
}

inline void add_incoming_edges_from_new_route(
    std::map<trip::route_edge, trip::route_edge> const& edges,
    std::map<node const*, std::vector<edge*>>& in) {
  for (auto const& trp_e : edges) {
    auto e = trp_e.second.get_edge();
    in[e->to_].push_back(e);
  }
}

inline void rebuild_incoming_edges(
    std::set<station_node*> const& station_nodes,
    std::map<node const*, std::vector<edge*>> const& incoming) {
  for (auto& s : station_nodes) {
    auto const& s_in = incoming.at(s);
    s->incoming_edges_ = array<edge*>(begin(s_in), end(s_in));

    for (auto& station_edge : s->edges_) {
      auto const& n_in = incoming.at(station_edge.to_);
      station_edge.to_->incoming_edges_ = array<edge*>(begin(n_in), end(n_in));
    }
  }
}

}  // namespace rt
}  // namespace motis
