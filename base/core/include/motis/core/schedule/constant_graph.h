#pragma once

#include <queue>
#include <vector>
#include <unordered_map>
#include <array>
#include <functional>

#include "motis/core/schedule/nodes.h"

namespace motis {

struct simple_edge {
  simple_edge() : to(0) {}
  simple_edge(int to, edge_cost cost)
      : to(to),
        dist({{cost.time, static_cast<uint16_t>(cost.transfer ? 1 : 0),
               cost.price}}) {}

  bool operator<(const simple_edge& o) const {
    if (to < o.to) {
      return true;
    } else if (to == o.to) {
      return dist < o.dist;
    }
    return false;
  }

  uint32_t to;
  std::array<uint16_t, 3> dist;
};

class constant_graph {
public:
  constant_graph() = default;

  constant_graph(std::vector<station_node_ptr> const& station_nodes) {
    for (auto const& station_node : station_nodes) {
      add_edges(*station_node.get());
      for (auto const& station_node_edge : station_node->_edges) {
        add_edges(*station_node_edge.get_destination());
      }
    }
  }

  void add_edges(node const& node) {
    for (auto const& edge : node._edges) {
      auto from = edge.get_destination()->_id;

      if (_edges.size() <= from) {
        _edges.resize(from + 1);
      }

      _edges[from].emplace_back(node._id, edge.get_minimum_cost());
    }
  }

  int add_edge(int node_index, const simple_edge& new_edge) {
    _edges[node_index].push_back(new_edge);
    return _edges[node_index].size() - 1;
  }

  std::vector<std::vector<simple_edge>> _edges;
};

template <int Criterion>
class constant_graph_dijkstra {
public:
  struct label {
    label(uint32_t node, uint32_t dist) : node(node), dist(dist) {}

    friend bool operator>(label const& a, label const& b) {
      return a.dist > b.dist;
    }

    uint32_t node, dist;
  };

  constant_graph_dijkstra(
      constant_graph const& graph, int goal,
      std::unordered_map<int, std::vector<simple_edge>> const& additional_edges)
      : _graph(graph), _additional_edges(additional_edges) {
    _dists.resize(_graph._edges.size(), std::numeric_limits<uint32_t>::max());
    _dists[goal] = 0;
    _pq.push(label(goal, 0));
  }

  inline uint32_t operator[](int node) const {
    assert(node < static_cast<int>(_dists.size()));
    return _dists[node];
  }

  void run() {
    while (!_pq.empty()) {
      auto label = _pq.top();
      _pq.pop();

      for (auto const& edge : _graph._edges[label.node]) {
        expand_edge(label.dist, edge);
      }

      auto additional_edges_it = _additional_edges.find(label.node);
      if (additional_edges_it != std::end(_additional_edges)) {
        for (auto const& edge : additional_edges_it->second) {
          expand_edge(label.dist, edge);
        }
      }
    }
  }

  inline void expand_edge(uint32_t dist, simple_edge const& edge) {
    uint32_t new_dist = dist + edge.dist[Criterion];
    if (new_dist < _dists[edge.to]) {
      _dists[edge.to] = new_dist;
      _pq.push(label(edge.to, new_dist));
    }
  }

  constant_graph const& _graph;
  std::priority_queue<label, std::vector<label>, std::greater<label>> _pq;
  std::vector<uint32_t> _dists;
  std::unordered_map<int, std::vector<simple_edge>> const& _additional_edges;
};

}  // namespace motis
