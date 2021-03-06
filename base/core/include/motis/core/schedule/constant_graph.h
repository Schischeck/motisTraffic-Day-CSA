#pragma once

#include <array>
#include <functional>
#include <queue>
#include <vector>

#include "motis/core/common/dial.h"
#include "motis/core/common/hash_map.h"
#include "motis/core/schedule/nodes.h"

namespace motis {

//=============================================================================
// GRAPH DEFINITION
//-----------------------------------------------------------------------------
struct simple_edge {
  simple_edge(uint32_t to, uint16_t cost) : to_(to), cost_(cost) {}
  uint32_t to_;
  uint16_t cost_;
};

using constant_graph = std::vector<std::vector<simple_edge>>;

//=============================================================================
// STATION GRAPH
//-----------------------------------------------------------------------------
struct map_station_graph_node {
  uint32_t operator()(node const* n) const { return n->get_station()->id_; }
};

inline constant_graph build_station_graph(
    std::vector<station_node_ptr> const& station_nodes, search_dir const dir) {
  constant_graph g(station_nodes.size());

  auto add_edges = [&g, dir](station_node const& sn) {
    // Stores the minimum distance to each neighboring station.
    std::unordered_map<uint32_t /* neighbor station node id */, uint32_t> min;

    auto update_min = [&min](uint32_t const from, edge_cost const& ec) {
      if (!ec.is_valid()) {
        return;
      }

      auto const it = min.find(from);
      if (it == end(min) || ec.time_ < it->second) {
        min[from] = ec.time_.ts();
      }
    };

    for (auto const& inner_station_edge : sn.edges_) {
      for (auto const& e : inner_station_edge.to_->edges_) {
        if (e.to_->get_station() != &sn) {
          update_min(e.to_->get_station()->id_, e.get_minimum_cost());
        }
      }
    }

    for (const auto& e : min) {
      auto const s = (dir == search_dir::FWD) ? e.first : sn.id_;
      auto const t = (dir == search_dir::FWD) ? sn.id_ : e.first;
      g[s].emplace_back(t, e.second);
    }
  };

  for (auto const& station_node : station_nodes) {
    add_edges(*station_node);
  }

  return g;
}

//=============================================================================
// INTERCHANGE GRAPH
//-----------------------------------------------------------------------------
struct map_interchange_graph_node {
  explicit map_interchange_graph_node(uint32_t route_offset)
      : route_offset_(route_offset) {}
  uint32_t operator()(node const* n) const {
    return n->is_route_node() ? route_offset_ + n->route_
                              : n->get_station()->id_;
  }
  uint32_t route_offset_;
};

inline bool is_connected(node const* from, node const* to) {
  return std::find_if(begin(from->edges_), end(from->edges_),
                      [&to](edge const& e) {
                        return e.to_ == to && e.type() != edge::INVALID_EDGE;
                      }) != end(from->edges_);
}

inline constant_graph build_interchange_graph(
    std::vector<station_node_ptr> const& station_nodes, unsigned route_count,
    search_dir const dir) {
  auto const route_offset = static_cast<uint32_t>(station_nodes.size());
  constant_graph g(route_offset + route_count);

  auto is_new = [&g, dir](uint32_t from, uint32_t to) {
    auto const s = (dir == search_dir::FWD) ? to : from;
    auto const t = (dir == search_dir::FWD) ? from : to;
    return std::find_if(begin(g[s]), end(g[s]), [&t](simple_edge const& e) {
             return e.to_ == t;
           }) == end(g[s]);
  };

  auto add_edge = [&g, is_new, dir](uint32_t from, uint32_t to,
                                    bool is_transfer) {
    if (is_new(from, to)) {
      auto const s = (dir == search_dir::FWD) ? to : from;
      auto const t = (dir == search_dir::FWD) ? from : to;
      g[s].emplace_back(t, is_transfer);
    }
  };

  auto add_station_edges = [&g, route_offset,
                            add_edge](station_node const* sn) {
    for (auto const& e : sn->edges_) {
      if (e.to_->is_foot_node()) {
        for (auto const& fe : e.to_->edges_) {
          if (fe.to_->is_station_node()) {
            g[fe.to_->id_].emplace_back(sn->id_, false);
          }
        }
      } else if (e.to_->is_route_node()) {
        auto const route_lb_node_id = e.to_->route_ + route_offset;

        if (is_connected(sn, e.to_)) {
          add_edge(sn->id_, route_lb_node_id, false);
        }

        if (is_connected(e.to_, sn)) {
          add_edge(route_lb_node_id, sn->id_, true);
        }
      }
    }
  };

  for (auto const& sn : station_nodes) {
    add_station_edges(sn.get());
  }

  return g;
}

//=============================================================================
// DIJKSTRA
//-----------------------------------------------------------------------------
template <uint32_t MaxValue, typename MapNodeFn>
class constant_graph_dijkstra {
public:
  using dist_t = uint32_t;

  struct label {
    label(uint32_t node, uint32_t dist) : node_(node), dist_(dist) {}

    friend bool operator>(label const& a, label const& b) {
      return a.dist_ > b.dist_;
    }

    uint32_t node_;
    dist_t dist_;
  };

  struct get_bucket {
    std::size_t operator()(label const& l) const { return l.dist_; }
  };

  enum : dist_t { UNREACHABLE = std::numeric_limits<dist_t>::max() };

  constant_graph_dijkstra(
      constant_graph const& g, int goal,
      hash_map<int, std::vector<simple_edge>> const& additional_edges,
      MapNodeFn map_node = MapNodeFn())
      : graph_(g),
        additional_edges_(additional_edges),
        map_node_(std::forward<MapNodeFn>(map_node)) {
    dists_.resize(graph_.size(), UNREACHABLE);
    dists_[goal] = 0;
    pq_.push(label(goal, 0));
  }

  inline dist_t operator[](node const* n) const {
    auto const idx = map_node_(n);
    assert(idx < dists_.size());
    return dists_[idx];
  }

  void run() {
    while (!pq_.empty()) {
      auto label = pq_.top();
      pq_.pop();

      for (auto const& edge : graph_[label.node_]) {
        expand_edge(label.dist_, edge);
      }

      auto additional_edges_it = additional_edges_.find(label.node_);
      if (additional_edges_it != std::end(additional_edges_)) {
        for (auto const& edge : additional_edges_it->second) {
          expand_edge(label.dist_, edge);
        }
      }
    }
  }

  inline void expand_edge(uint32_t dist, simple_edge const& edge) {
    uint32_t new_dist = dist + edge.cost_;  // NOLINT
    if (new_dist < dists_[edge.to_] && new_dist <= MaxValue) {
      dists_[edge.to_] = new_dist;
      pq_.push(label(edge.to_, new_dist));
    }
  }

  inline bool is_reachable(dist_t val) { return val != UNREACHABLE; }

  std::vector<std::vector<simple_edge>> const& graph_;
  dial<label, MaxValue, get_bucket> pq_;
  std::vector<dist_t> dists_;
  hash_map<int, std::vector<simple_edge>> const& additional_edges_;
  MapNodeFn map_node_;
};

}  // namespace motis
