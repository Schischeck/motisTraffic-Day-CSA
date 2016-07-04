#pragma once

#include "motis/core/common/get_or_create.h"
#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/schedule.h"
#include "motis/loader/util.h"

#include "motis/rt/bfs.h"

namespace motis {
namespace rt {

inline std::set<edge const*> route_edges(trip const* trp) {
  auto const first_e = trp->edges_->at(0).get_edge();
  auto const first_dep = ev_key{first_e, trp->lcon_idx_, event_type::DEP};
  return route_bfs(first_dep, bfs_direction::BOTH, true);
}

inline bool get_in_allowed(node const* n) {
  for (auto const& e : n->incoming_edges_) {
    if (e->from_ == n->get_station()) {
      return true;
    }
  }
  return false;
}

inline bool get_out_allowed(node const* n) {
  for (auto const& e : n->edges_) {
    if (e.get_destination() == n->get_station()) {
      return true;
    }
  }
  return false;
}

inline std::pair<bool, bool> get_in_out_allowed(node const* n) {
  return std::make_pair(get_in_allowed(n), get_out_allowed(n));
}

inline std::map<node const*, std::pair<bool, bool>> get_route_in_out_allowed(
    trip const* trp) {
  std::map<node const*, std::pair<bool, bool>> in_out_allowed;
  for (auto const& e : route_edges(trp)) {
    in_out_allowed[e->from_] = get_in_out_allowed(e->from_);
    in_out_allowed[e->to_] = get_in_out_allowed(e->to_);
  }
  return in_out_allowed;
}

inline edge copy_edge(edge const& original, node* from, node* to,
                      int lcon_index) {
  edge e;
  if (original.type() == edge::ROUTE_EDGE) {
    return make_route_edge(from, to,
                           {original.m_.route_edge_.conns_[lcon_index]});
  } else {
    e = original;
  }
  e.from_ = from;
  e.to_ = to;
  return e;
}

inline void copy_trip_route(schedule& sched, trip const* trp,
                            std::map<node const*, node*>& nodes,
                            std::map<edge const*, trip::route_edge>& edges) {
  auto const build_node = [&](node const* orig) {
    auto n = new node(orig->station_node_, sched.node_count_++);
    n->route_ = orig->route_;
    return n;
  };

  for (auto const& e : route_edges(trp)) {
    auto from = get_or_create<node const*, node*>(
        nodes, e->from_, [&]() { return build_node(e->from_); });
    auto to = get_or_create<node const*, node*>(
        nodes, e->to_, [&]() { return build_node(e->to_); });

    if (e->type() == edge::ROUTE_EDGE) {
      auto const& lcon = e->m_.route_edge_.conns_[trp->lcon_idx_];
      const_cast<light_connection&>(lcon).valid_ = false;  // NOLINT
    }

    from->edges_.push_back(copy_edge(*e, from, to, trp->lcon_idx_));
    edges[e] = trip::route_edge(&from->edges_.back());
  }
}

inline std::set<trip const*> route_trips(schedule const& sched,
                                         trip const* trp) {
  std::set<trip const*> trips;
  for (auto const& e : route_edges(trp)) {
    if (e->empty()) {
      continue;
    }

    auto trips_idx = e->m_.route_edge_.conns_[trp->lcon_idx_].trips_;
    auto const& merged_trips = *sched.merged_trips_.at(trips_idx);
    trips.insert(begin(merged_trips), end(merged_trips));
  }
  return trips;
}

inline void update_trips(schedule& sched, trip const* trp,
                         std::map<edge const*, trip::route_edge>& edges) {
  for (auto const& t : route_trips(sched, trp)) {
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
    std::map<node const*, std::pair<bool, bool>> const& in_out_allowed,
    std::map<node const*, node*>& nodes) {
  for (auto& n : nodes) {
    auto station_node =
        sched.station_nodes_.at(n.first->get_station()->id_).get();
    auto route_node = n.second;

    if (!in_out_allowed.at(n.first).first) {
      station_node->edges_.push_back(
          make_invalid_edge(station_node, route_node));
    } else {
      station_node->edges_.push_back(make_foot_edge(station_node, route_node));
    }

    if (!in_out_allowed.at(n.first).second) {
      route_node->edges_.push_back(make_invalid_edge(route_node, station_node));
    } else {
      route_node->edges_.push_back(make_foot_edge(
          route_node, station_node,
          sched.stations_.at(station_node->id_)->transfer_time_, true));
    }

    if (in_out_allowed.at(n.first).second && station_node->foot_node_) {
      route_node->edges_.push_back(
          make_after_train_edge(route_node, station_node->foot_node_));
    }
  }
}

inline void seperate_trip(schedule& sched, trip const* trp) {
  std::map<node const*, node*> nodes;
  std::map<edge const*, trip::route_edge> edges;
  auto in_out_allowed = get_route_in_out_allowed(trp);

  copy_trip_route(sched, trp, nodes, edges);
  update_trips(sched, trp, edges);
  build_change_edges(sched, in_out_allowed, nodes);
}

}  // namespace rt
}  // namespace motis
