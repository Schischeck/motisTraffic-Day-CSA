#include "motis/path/prepare/rail/rail_router.h"

#include <queue>

#include "utl/erase.h"
#include "utl/erase_duplicates.h"
#include "utl/to_vec.h"
#include "utl/repeat_n.h"

#include "motis/core/common/logging.h"

namespace motis {
namespace path {

constexpr auto kUnreached = std::numeric_limits<size_t>::max();

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

  rail_graph_dijkstra(rail_graph const& graph) : graph_(graph) {
    dists_ = utl::repeat_n(kUnreached, graph_.nodes_.size());
    edges_ = utl::repeat_n<rail_edge const*>(nullptr, graph_.nodes_.size());
  }

  void initialize(rail_phantom const& phantom) {
    auto const& info = graph_.infos_[phantom.info_idx_];

    // edge: bw --> phantom --> fw

    auto const bw_dist = phantom.dist_;
    auto const bw_node_idx = info->from_idx();
    dists_[bw_node_idx] = bw_dist;
    pq_.emplace(bw_node_idx, bw_dist, nullptr);

    auto const fw_dist = info->dist_ - phantom.dist_;
    auto const fw_node_idx = info->to_idx();
    dists_[fw_node_idx] = fw_dist;
    pq_.emplace(fw_node_idx, fw_dist, nullptr);
  }

  void set_goals(std::vector<rail_phantom> const& phantoms) {
    for (auto const& phantom : phantoms) {
      auto const& info = graph_.infos_[phantom.info_idx_];

      if (phantom.dist_ != 0) {
        open_goals_.push_back(info->to_idx());
      }

      if (phantom.dist_ != info->dist_) {
        open_goals_.push_back(info->from_idx());
      }
    }

    utl::erase_duplicates(open_goals_);
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
        if (new_dist < dists_[to_idx]) {
          dists_[to_idx] = new_dist;
          edges_[to_idx] = &edge;
          pq_.push({to_idx, new_dist, &edge});
        }
      }
    }
  }

  rail_path get_path(rail_phantom const& p_source,
                     rail_phantom const& p_goal) const {
    auto const& info = graph_.infos_[p_goal.info_idx_];

    rail_path path;
    path.source_ = &p_source;
    path.goal_ = &p_goal;

    size_t used_goal;
    if (p_goal.dist_ == 0) {
      used_goal = info->from_idx();
    } else if (p_goal.dist_ == info->dist_) {
      used_goal = info->to_idx();
    } else {
      used_goal = (dists_[info->from_idx()] < dists_[info->from_idx()])
                      ? info->from_idx()
                      : info->to_idx();
    }

    path.valid_ = dists_[used_goal] != kUnreached;

    auto edge = edges_[used_goal];
    while (edge != nullptr) {
      path.edges_.push_back(edge);
      edge = edges_[edge->from_->idx_];
    }
    std::reverse(begin(path.edges_), end(path.edges_));

    return path;
  }

  rail_graph const& graph_;
  std::priority_queue<label, std::vector<label>, std::greater<label>> pq_;

  std::vector<size_t> dists_;
  std::vector<rail_edge const*> edges_;

  std::vector<size_t> open_goals_;
};

std::vector<rail_path> shortest_paths(rail_graph const& graph,
                                      rail_phantom const& from,
                                      std::vector<rail_phantom> const& tos) {
  rail_graph_dijkstra dijkstra{graph};
  dijkstra.initialize(from);
  dijkstra.set_goals(tos);
  dijkstra.run();

  return utl::to_vec(
      tos, [&](auto const& to) { return dijkstra.get_path(from, to); });
}

}  // namespace path
}  // namespace motis
