#pragma once

#include "utl/get_or_create.h"
#include "utl/to_vec.h"

#include "motis/core/common/hash_map.h"
#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/access/bfs.h"

#include "motis/rt/in_out_allowed.h"
#include "motis/rt/incoming_edges.h"

#include "motis/core/access/service_access.h"

namespace motis {
namespace rt {

inline std::set<trip::route_edge> route_edges(ev_key const& k) {
  return route_bfs(k, bfs_direction::BOTH, true);
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

inline void copy_trip_route(
    schedule& sched, ev_key const& k, std::map<node const*, node*>& nodes,
    std::map<trip::route_edge, trip::route_edge>& edges) {
  auto const route_id = sched.route_count_++;

  auto const build_node = [&](node const* orig) {
    return new node(orig->station_node_, sched.node_count_++,  // NOLINT
                    route_id);
  };

  for (auto const& e : route_edges(k)) {
    auto const from = utl::get_or_create(nodes, e->from_,
                                         [&] { return build_node(e->from_); });
    auto const to =
        utl::get_or_create(nodes, e->to_, [&] { return build_node(e->to_); });

    from->edges_.push_back(copy_edge(*e, from, to, k.lcon_idx_));
    edges[e] = trip::route_edge(&from->edges_.back());

    if (e->type() == edge::ROUTE_EDGE) {
      auto const& lcon = e->m_.route_edge_.conns_[k.lcon_idx_];
      const_cast<light_connection&>(lcon).valid_ = false;  // NOLINT
    }
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
                         std::map<trip::route_edge, trip::route_edge>& edges) {
  for (auto const& t : route_trips(sched, k)) {
    sched.trip_edges_.emplace_back(
        std::make_unique<std::vector<trip::route_edge>>(
            utl::to_vec(*t->edges_, [&](trip::route_edge const& e) {
              return edges.at(e.get_edge());
            })));
    const_cast<trip*>(t)->edges_ = sched.trip_edges_.back().get();  // NOLINT
    const_cast<trip*>(t)->lcon_idx_ = 0;  // NOLINT
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

    if (in_out.out_allowed_ && station_node->foot_node_ != nullptr) {
      route_node->edges_.push_back(
          make_after_train_edge(route_node, station_node->foot_node_, 0, true));
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

inline void update_delays(
    std::size_t lcon_idx,
    std::map<trip::route_edge, trip::route_edge> const& edges,
    schedule& sched) {
  auto const update_di = [&](ev_key const& orig_k, ev_key const& new_k) {
    auto const it = sched.graph_to_delay_info_.find(orig_k);
    if (it != end(sched.graph_to_delay_info_)) {
      auto const di = it->second;
      sched.graph_to_delay_info_[new_k] = di;
      di->set_ev_key(new_k);
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

inline void seperate_trip(schedule& sched, ev_key const& k) {
  auto const in_out_allowed = get_route_in_out_allowed(k);
  auto const station_nodes = route_station_nodes(k);
  auto incoming = incoming_non_station_edges(station_nodes);
  auto nodes = std::map<node const*, node*>{};
  auto edges = std::map<trip::route_edge, trip::route_edge>{};

  copy_trip_route(sched, k, nodes, edges);
  update_trips(sched, k, edges);
  build_change_edges(sched, in_out_allowed, nodes);
  add_incoming_station_edges(station_nodes, incoming);
  add_incoming_edges_from_new_route(edges, incoming);
  rebuild_incoming_edges(station_nodes, incoming);
  update_delays(k.lcon_idx_, edges, sched);
}

inline void seperate_trip(schedule& sched, trip const* trp) {
  auto const first_dep =
      ev_key{trp->edges_->front().get_edge(), trp->lcon_idx_, event_type::DEP};
  seperate_trip(sched, first_dep);
}

}  // namespace rt
}  // namespace motis
