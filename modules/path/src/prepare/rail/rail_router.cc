#include "motis/path/prepare/rail/rail_router.h"

#include <queue>

#include "geo/latlng.h"

#include "utl/erase.h"
#include "utl/to_vec.h"

#include "motis/core/common/logging.h"

namespace motis {
namespace path {

struct rail_graph_dijkstra {
  struct label {
    label(size_t const idx, size_t const dist, size_t const est_dist,
          rail_link const* link)
        : idx_(idx), dist_(dist), est_dist_(est_dist), link_(link) {}

    friend bool operator>(label const& a, label const& b) {
      return a.est_dist_ > b.est_dist_;
    }

    size_t idx_, dist_, est_dist_;
    rail_link const* link_;
  };

  rail_graph_dijkstra(rail_graph const& graph, std::vector<size_t> initial,
                      std::vector<size_t> goals)
      : graph_(graph), goals_(goals), open_goals_(goals) {
    dists_.resize(graph_.nodes_.size(), std::numeric_limits<size_t>::max());
    links_.resize(graph_.nodes_.size(), nullptr);

    if (!goals.empty()) {
      for (auto const& i : initial) {
        dists_[i] = 0;
        pq_.push(label(i, 0, est_dist(i), nullptr));
      }
    }
  }

  size_t est_dist(size_t const this_idx) {
    auto const est = utl::to_vec(goals_, [&](auto const& goal_idx) {
      return distance(graph_.nodes_[this_idx]->pos_,
                      graph_.nodes_[goal_idx]->pos_);
    });
    return *std::min_element(begin(est), end(est));
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
      for (auto const& link : node->links_) {
        size_t const new_dist = label.dist_ + link.dist_;
        size_t const to_idx = link.to_->idx_;
        if (new_dist < dists_[to_idx]) {
          dists_[to_idx] = new_dist;
          links_[to_idx] = &link;
          pq_.emplace(to_idx, new_dist, est_dist(this_idx), &link);
        }
      }
    }
  }

  std::vector<rail_link const*> get_links(size_t const goal) const {
    std::vector<rail_link const*> result;

    auto link = links_[goal];
    while (link != nullptr) {
      result.push_back(link);
      link = links_[link->from_->idx_];
    }

    std::reverse(begin(result), end(result));
    return result;
  }

  size_t get_distance(size_t const goal) const { return dists_[goal]; }

  rail_graph const& graph_;
  std::priority_queue<label, std::vector<label>, std::greater<label>> pq_;

  std::vector<size_t> dists_;
  std::vector<rail_link const*> links_;

  std::vector<size_t> goals_;
  std::vector<size_t> open_goals_;
};

std::vector<std::vector<rail_link const*>> shortest_paths(
    rail_graph const& graph, std::vector<size_t> const& from,
    std::vector<size_t> const& to) {

  rail_graph_dijkstra dijkstra{graph, from, to};
  dijkstra.run();

  return utl::to_vec(to,
                     [&](auto const& id) { return dijkstra.get_links(id); });
}

}  // namespace path
}  // namespace motis
