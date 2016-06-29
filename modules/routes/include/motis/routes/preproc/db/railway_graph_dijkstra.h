#pragma once

#include "motis/routes/preproc/db/railway_graph.h"

namespace motis {
namespace routes {

struct railway_graph_dijkstra {
  struct label {
    label(size_t const idx, size_t const dist, railway_link const* link)
        : idx_(idx), dist_(dist), link_(link) {}

    friend bool operator>(label const& a, label const& b) {
      return a.dist_ > b.dist_;
    }

    size_t idx_, dist_;
    railway_link const* link_;
  };

  railway_graph_dijkstra(railway_graph const& graph, size_t from,
                         std::vector<size_t> goals)
      : graph_(graph), goals_(goals), open_goals_(goals) {
    dists_.resize(graph_.nodes_.size(), std::numeric_limits<size_t>::max());
    links_.resize(graph_.nodes_.size(), nullptr);

    dists_[from] = 0;
    pq_.push(label(from, 0, nullptr));
  }

  void run() {
    while (!pq_.empty()) {
      auto label = pq_.top();
      pq_.pop();

      auto const this_idx = label.idx_;

      open_goals_.erase(
          std::remove(begin(open_goals_), end(open_goals_), this_idx),
          end(open_goals_));
      if (open_goals_.empty()) {
        break;
      }

      auto const& node = graph_.nodes_[this_idx];
      for (auto const& link : node->links_) {
        size_t const new_dist = label.dist_ + link->dist_;
        size_t const to_idx = link->to_->idx_;
        if (new_dist < dists_[to_idx]) {
          dists_[to_idx] = new_dist;
          links_[to_idx] = link;
          pq_.push({to_idx, new_dist, link});
        }
      }
    }
  }

  std::vector<railway_link const*> get_links(size_t const goal) const {
    std::vector<railway_link const*> result;

    auto link = links_[goal];
    while(link != nullptr) {
      result.push_back(link);
      link = links_[link->from_->idx_];
    }

    std::reverse(begin(result), end(result));
    return result;
  }

  railway_graph const& graph_;
  std::priority_queue<label, std::vector<label>, std::greater<label>> pq_;

  std::vector<size_t> dists_;
  std::vector<railway_link const*> links_;

  std::vector<size_t> goals_;
  std::vector<size_t> open_goals_;
};

}  // namespace routes
}  // namespace motis