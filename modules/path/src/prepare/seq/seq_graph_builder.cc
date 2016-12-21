#include "motis/path/prepare/seq/seq_graph_builder.h"

#include "utl/to_vec.h"

using namespace geo;

namespace motis {
namespace path {

void add_close_nodes(seq_graph& g, station_seq const& seq,
                     routing_strategy* routing_strategy) {
  for (auto i = 0u; i < seq.station_ids_.size(); ++i) {
    auto it = std::find(begin(seq.station_ids_), end(seq.station_ids_),
                        seq.station_ids_[i]);
    if (it == end(seq.station_ids_)) {
      continue;
    }
    for (auto const& node : routing_strategy->close_nodes(
             seq.coordinates_[std::distance(begin(seq.station_ids_), it)])) {
      g.nodes_.emplace_back(
          std::make_unique<seq_node>(g.nodes_.size(), i, node));
      g.station_to_nodes_[i].push_back(g.nodes_.back().get());
    }
  }
}

void connect_nodes(std::vector<seq_node*>& from_nodes,
                   std::vector<seq_node*> const& to_nodes,
                   std::vector<std::vector<routing_result>> const& results) {
  for (auto i = 0u; i < from_nodes.size(); ++i) {
    for (auto j = 0u; j < to_nodes.size(); ++j) {
      auto& from = from_nodes[i];
      auto const& to = to_nodes[j];

      if (from->idx_ == to->idx_) {
        continue;
      }

      if (std::find_if(begin(from->edges_), end(from->edges_),
                       [&](auto const& e) {
                         return e.to_->idx_ == to->idx_;
                       }) != end(from->edges_) ||
          results[i].size() <= j || results[i].size() == 0) {
        continue;
      }
      from->edges_.emplace_back(from, to, results[i][j]);
    }
  }
}

void create_edges(seq_graph& g, routing_strategy* routing_strategy) {
  auto refs = utl::to_vec(g.station_to_nodes_, [](auto const& sn) {
    return utl::to_vec(sn, [](auto const& node) { return node->ref_; });
  });

  for (auto i = 0u; i < g.station_to_nodes_.size() - 1; ++i) {
    if (i != 0) {
      connect_nodes(g.station_to_nodes_[i], g.station_to_nodes_[i],
                    routing_strategy->find_routes(refs[i], refs[i]));
    }

    connect_nodes(g.station_to_nodes_[i], g.station_to_nodes_[i + 1],
                  routing_strategy->find_routes(refs[i], refs[i + 1]));
  }
}

seq_graph build_seq_graph(
    station_seq const& seq,
    std::vector<routing_strategy*> const& routing_strategies) {
  seq_graph g{seq.station_ids_.size()};

  for (auto const& s : routing_strategies) {
    add_close_nodes(g, seq, s);
    create_edges(g, s);
  }

  g.initials_ = utl::to_vec(g.station_to_nodes_.front(),
                            [](auto&& node) { return node->idx_; });
  g.goals_ = utl::to_vec(g.station_to_nodes_.back(),
                         [](auto&& node) { return node->idx_; });
  return g;
}

}  // namespace motis
}  // namespace path
