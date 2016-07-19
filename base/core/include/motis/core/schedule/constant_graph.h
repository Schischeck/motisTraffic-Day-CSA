#pragma once

#include <array>
#include <functional>
#include <queue>
#include <unordered_map>
#include <vector>

#include "motis/core/schedule/nodes.h"

namespace motis {

struct simple_edge {
  simple_edge() : to_(0) {}
  simple_edge(int to, edge_cost cost)
      : to_(to),
        dist_({{cost.time_, static_cast<uint16_t>(cost.transfer_ ? 1 : 0),
                cost.price_}}) {}

  bool operator<(const simple_edge& o) const {
    if (to_ < o.to_) {
      return true;
    } else if (to_ == o.to_) {
      return dist_ < o.dist_;
    }
    return false;
  }

  uint32_t to_;
  std::array<uint16_t, 3> dist_;
};

class constant_graph {
public:
  constant_graph() = default;

  explicit constant_graph(std::vector<station_node_ptr> const& station_nodes,
                          search_dir const dir) {
    for (auto const& station_node : station_nodes) {
      add_edges(*station_node, dir);
      for (auto const& station_node_edge : station_node->edges_) {
        add_edges(*station_node_edge.get_destination(), dir);
      }
    }
  }

  void add_edges(node const& node, search_dir const dir) {
    for (auto const& edge : node.edges_) {
      auto const from = edge.get_destination(dir)->id_;
      auto const to = edge.get_source(dir)->id_;

      if (edges_.size() <= from) {
        edges_.resize(from + 1);
      }

      edges_[from].emplace_back(to, edge.get_minimum_cost());
    }
  }

  int add_edge(int node_index, const simple_edge& new_edge) {
    edges_[node_index].push_back(new_edge);
    return edges_[node_index].size() - 1;
  }

  std::vector<std::vector<simple_edge>> edges_;
};

template <int Criterion>
class constant_graph_dijkstra {
public:
  struct label {
    label(uint32_t node, uint32_t dist) : node_(node), dist_(dist) {}

    friend bool operator>(label const& a, label const& b) {
      return a.dist_ > b.dist_;
    }

    uint32_t node_, dist_;
  };

  constant_graph_dijkstra(
      constant_graph const& graph, int goal,
      std::unordered_map<int, std::vector<simple_edge>> const& additional_edges)
      : graph_(graph), additional_edges_(additional_edges) {
    dists_.resize(graph_.edges_.size(), std::numeric_limits<uint32_t>::max());
    dists_[goal] = 0;
    pq_.push(label(goal, 0));
  }

  inline uint32_t operator[](int node) const {
    assert(node < static_cast<int>(dists_.size()));
    return dists_[node];
  }

  void run() {
    while (!pq_.empty()) {
      auto label = pq_.top();
      pq_.pop();

      for (auto const& edge : graph_.edges_[label.node_]) {
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
    uint32_t new_dist = dist + edge.dist_[Criterion];  // NOLINT
    if (new_dist < dists_[edge.to_]) {
      dists_[edge.to_] = new_dist;
      pq_.push(label(edge.to_, new_dist));
    }
  }

  constant_graph const& graph_;
  std::priority_queue<label, std::vector<label>, std::greater<label>> pq_;
  std::vector<uint32_t> dists_;
  std::unordered_map<int, std::vector<simple_edge>> const& additional_edges_;
};

}  // namespace motis
