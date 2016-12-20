#include "motis/path/prepare/rail/rail_router.h"

#include <queue>

#include "common/erase.h"

#include "motis/core/common/logging.h"
#include "motis/core/common/transform_to_vec.h"

using namespace common;

namespace motis {
namespace path {

struct rail_graph_dijkstra {
  struct label {
    label(size_t const idx, size_t const dist, rail_link const* link)
        : idx_(idx), dist_(dist), link_(link) {}

    friend bool operator>(label const& a, label const& b) {
      return a.dist_ > b.dist_;
    }

    size_t idx_, dist_;
    rail_link const* link_;
  };

  rail_graph_dijkstra(rail_graph const& graph, std::vector<size_t> initial,
                      std::vector<size_t> goals)
      : graph_(graph), goals_(goals), open_goals_(goals) {
    dists_.resize(graph_.nodes_.size(), std::numeric_limits<size_t>::max());
    links_.resize(graph_.nodes_.size(), nullptr);

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

      erase(open_goals_, this_idx);
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
          pq_.push({to_idx, new_dist, &link});
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

  return transform_to_vec(
      to, [&](auto const& id) { return dijkstra.get_links(id); });
}

}  // namespace path
}  // namespace motis
