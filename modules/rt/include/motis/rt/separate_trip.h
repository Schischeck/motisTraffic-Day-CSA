#pragma once

#include "motis/core/common/get_or_create.h"
#include "motis/core/common/hash_map.h"
#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/schedule.h"
#include "motis/loader/util.h"

#include "motis/rt/bfs.h"

#include "motis/core/access/service_access.h"

namespace motis {
namespace rt {

struct in_out_allowed {
  in_out_allowed() = default;
  in_out_allowed(bool in_allowed, bool out_allowed)
      : in_allowed_(in_allowed), out_allowed_(out_allowed) {}
  bool in_allowed_, out_allowed_;
};

inline std::set<edge const*> route_edges(ev_key const& k) {
  return route_bfs(k, bfs_direction::BOTH, true);
}

inline bool get_in_allowed(node const* n) {
  for (auto const& e : n->incoming_edges_) {
    if (e->from_ == n->get_station() && e->type() != edge::INVALID_EDGE) {
      return true;
    }
  }
  return false;
}

inline bool get_out_allowed(node const* n) {
  for (auto const& e : n->edges_) {
    if (e.get_destination() == n->get_station() &&
        e.type() != edge::INVALID_EDGE) {
      return true;
    }
  }
  return false;
}

inline in_out_allowed get_in_out_allowed(node const* n) {
  return {get_in_allowed(n), get_out_allowed(n)};
}

inline std::map<node const*, in_out_allowed> get_route_in_out_allowed(
    ev_key const& k) {
  std::map<node const*, in_out_allowed> in_out_allowed;
  for (auto const& e : route_edges(k)) {
    in_out_allowed[e->from_] = get_in_out_allowed(e->from_);
    in_out_allowed[e->to_] = get_in_out_allowed(e->to_);
  }
  return in_out_allowed;
}

inline edge copy_edge(edge const& original, node* from, node* to,
                      int lcon_index) {
  edge e;
  if (original.type() == edge::ROUTE_EDGE) {
    e = make_route_edge(from, to, {original.m_.route_edge_.conns_[lcon_index]});
  } else {
    e = original;
    e.from_ = from;
    e.to_ = to;
  }
  return e;
}

inline void copy_trip_route(schedule& sched, ev_key const& k,
                            std::map<node const*, node*>& nodes,
                            std::map<edge const*, trip::route_edge>& edges) {
  auto const route_id = sched.route_count_++;

  auto const build_node = [&](node const* orig) {
    return new node(orig->station_node_, sched.node_count_++, route_id);
  };

  for (auto const& e : route_edges(k)) {
    auto from = get_or_create<node const*, node*>(
        nodes, e->from_, [&] { return build_node(e->from_); });
    auto to = get_or_create<node const*, node*>(
        nodes, e->to_, [&] { return build_node(e->to_); });

    if (e->type() == edge::ROUTE_EDGE) {
      auto const& lcon = e->m_.route_edge_.conns_[k.lcon_idx_];
      const_cast<light_connection&>(lcon).valid_ = false;  // NOLINT
    }

    from->edges_.push_back(copy_edge(*e, from, to, k.lcon_idx_));
    edges[e] = trip::route_edge(&from->edges_.back());
  }
}

inline std::set<trip const*> route_trips(schedule const& sched,
                                         ev_key const& k) {
  auto trips = std::set<trip const*>{};
  for (auto const& e : route_edges(k)) {
    if (e->empty()) {
      continue;
    }

    auto trips_idx = e->m_.route_edge_.conns_[k.lcon_idx_].trips_;
    auto const& merged_trips = *sched.merged_trips_.at(trips_idx);
    trips.insert(begin(merged_trips), end(merged_trips));
  }
  return trips;
}

inline void update_trips(schedule& sched, ev_key const& k,
                         std::map<edge const*, trip::route_edge>& edges) {
  for (auto const& t : route_trips(sched, k)) {
    sched.trip_edges_.emplace_back(
        std::make_unique<std::vector<trip::route_edge>>(
            loader::transform_to_vec(*t->edges_,
                                     [&](trip::route_edge const& e) {
                                       return edges.at(e.get_edge());
                                     })));
    const_cast<trip*>(t)->edges_ = sched.trip_edges_.back().get();  // NOLINT
  }
}

inline void build_change_edges(
    schedule& sched,
    std::map<node const*, in_out_allowed> const& in_out_allowed,
    std::map<node const*, node*> const& nodes) {
  for (auto& n : nodes) {
    auto station_node =
        sched.station_nodes_.at(n.first->get_station()->id_).get();
    auto in_out = in_out_allowed.at(n.first);
    auto route_node = n.second;

    if (!in_out.in_allowed_) {
      station_node->edges_.push_back(
          make_invalid_edge(station_node, route_node));
    } else {
      station_node->edges_.push_back(make_foot_edge(station_node, route_node));
    }

    if (!in_out.out_allowed_) {
      route_node->edges_.push_back(make_invalid_edge(route_node, station_node));
    } else {
      route_node->edges_.push_back(make_foot_edge(
          route_node, station_node,
          sched.stations_.at(station_node->id_)->transfer_time_, true));
    }

    if (in_out.out_allowed_ && station_node->foot_node_) {
      route_node->edges_.push_back(
          make_after_train_edge(route_node, station_node->foot_node_));
    }
  }
}

inline std::set<station_node*> route_station_nodes(ev_key const& k) {
  std::set<station_node*> station_nodes;
  for (auto const& e : route_edges(k)) {
    station_nodes.insert(e->from_->get_station());
    station_nodes.insert(e->to_->get_station());
  }
  return station_nodes;
}

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

inline void add_incoming_edges_from_new_route()

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

inline void update_delays(std::size_t lcon_idx,
                          std::map<edge const*, trip::route_edge> const& edges,
                          schedule& sched,
                          hash_map<ev_key, ev_key>& moved_events) {
  auto const update_di = [&](ev_key const& orig_k, ev_key const& new_k) {
    moved_events[orig_k] = new_k;
    auto it = sched.graph_to_delay_info_.find(orig_k);
    if (it != end(sched.graph_to_delay_info_)) {
      sched.graph_to_delay_info_[new_k] = it->second;
    }
  };

  for (auto const& entry : edges) {
    auto const e = entry.first;
    auto const new_e = entry.second.get_edge();

    auto const orig_dep = ev_key{e, lcon_idx, event_type::DEP};
    auto const new_dep = ev_key{new_e, 0, event_type::DEP};
    auto const orig_arr = ev_key{e, lcon_idx, event_type::ARR};
    auto const new_arr = ev_key{new_e, 0, event_type::ARR};

    update_di(orig_dep, new_dep);
    update_di(orig_arr, new_arr);
  }
}

inline void seperate_trip(schedule& sched, ev_key const& k,
                          hash_map<ev_key, ev_key>& moved_events) {
  auto const in_out_allowed = get_route_in_out_allowed(k);
  auto const station_nodes = route_station_nodes(k);
  auto incoming = incoming_non_station_edges(station_nodes);
  auto nodes = std::map<node const*, node*>{};
  auto edges = std::map<edge const*, trip::route_edge>{};

  copy_trip_route(sched, k, nodes, edges);
  update_trips(sched, k, edges);
  build_change_edges(sched, in_out_allowed, nodes);
  add_incoming_station_edges(station_nodes, incoming);
  rebuild_incoming_edges(station_nodes, incoming);
  update_delays(k.lcon_idx_, edges, sched, moved_events);
}

}  // namespace rt
}  // namespace motis
