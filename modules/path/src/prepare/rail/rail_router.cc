#include "motis/path/prepare/rail/rail_router.h"

#include <queue>

#include "utl/erase.h"
#include "utl/to_vec.h"

#include "motis/core/common/logging.h"

namespace motis {
namespace path {

struct rail_graph_dijkstra {
  struct label {
    label(size_t const idx, size_t const dist, rail_edge const* edge)
        : idx_(idx), dist_(dist), edge_(edge) {}

    friend bool operator>(label const& a, label const& b) {
      return a.dist_ > b.dist_;
    }

    size_t idx_, dist_;
    rail_edge const* edge_;
  };

  rail_graph_dijkstra(rail_graph const& graph, std::vector<size_t> initial,
                      std::vector<size_t> goals)
      : graph_(graph), goals_(goals), open_goals_(goals) {
    for (auto const& i : initial) {
      for (auto const& g : goals) {
        auto const dist =
            geo::distance(graph_.nodes_[i]->pos_, graph_.nodes_[g]->pos_);
        if (dist > limit_) {
          limit_ = dist;
        }
      }
    }
    limit_ *= 10;

    dists_.resize(graph_.nodes_.size(), std::numeric_limits<size_t>::max());
    edges_.resize(graph_.nodes_.size(), nullptr);

    for (auto const& i : initial) {
      dists_[i] = 0;
      pq_.push(label(i, 0, nullptr));
    }
  }

  void run() {
    while (!pq_.empty()) {
      auto label = pq_.top();
      pq_.pop();

      auto const this_idx = label.idx_;

      utl::erase(open_goals_, this_idx);
      if (open_goals_.empty()) {
        break;
      }

      auto const& node = graph_.nodes_[this_idx];
      for (auto const& edge : node->edges_) {
        size_t const new_dist = label.dist_ + edge.dist_;
        size_t const to_idx = edge.to_->idx_;
        if (new_dist < limit_ && new_dist < dists_[to_idx]) {
          dists_[to_idx] = new_dist;
          edges_[to_idx] = &edge;
          pq_.push({to_idx, new_dist, &edge});
        }
      }
    }
  }

  std::vector<rail_edge const*> get_edges(size_t const goal) const {
    std::vector<rail_edge const*> result;

    auto edge = edges_[goal];
    while (edge != nullptr) {
      result.push_back(edge);
      edge = edges_[edge->from_->idx_];
    }

    std::reverse(begin(result), end(result));
    return result;
  }

  size_t get_distance(size_t const goal) const { return dists_[goal]; }

  rail_graph const& graph_;
  std::priority_queue<label, std::vector<label>, std::greater<>> pq_;

  std::vector<size_t> dists_;
  std::vector<rail_edge const*> edges_;

  std::vector<size_t> goals_;
  std::vector<size_t> open_goals_;

  size_t limit_ = 0;
};

std::vector<std::vector<rail_edge const*>> shortest_paths(
    rail_graph const& graph, std::vector<size_t> const& from,
    std::vector<size_t> const& to) {

  rail_graph_dijkstra dijkstra{graph, from, to};
  dijkstra.run();

  return utl::to_vec(to,
                     [&](auto const& id) { return dijkstra.get_edges(id); });
}

}  // namespace path
}  // namespace motis
