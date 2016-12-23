#pragma once

#include <iostream>

#include "utl/repeat_n.h"

#include "motis/path/prepare/schedule/station_sequences.h"
#include "motis/path/prepare/seq/seq_graph.h"

namespace motis {
namespace path {

inline void print_seq_path(std::vector<seq_edge const*> const& edges) {
  for (auto const& edge : edges) {
    auto const& from = *edge->from_;
    auto const& to = *edge->to_;
    std::cout << from.idx_ << " (" << from.station_idx_ << " @ "
              << from.ref_.strategy_id_ << ") -> " << to.idx_ << " ("
              << to.station_idx_ << " @ " << to.ref_.strategy_id_
              << ") = " << edge->weight() << " " << edge->strategy_id()
              << std::endl;
  }
}

inline void print_seq_graph(seq_graph const& g) {
  auto nodes = utl::repeat_n(std::vector<seq_node*>{}, g.seq_size_);
  for (auto& node : g.nodes_) {
    nodes[node->station_idx_].push_back(node.get());
  }

  std::cout << "\n\n";
  for (auto i = 0u; i < nodes.size(); ++i) {
    std::cout << "station: " << i << "\n";
    for (auto const& node : nodes[i]) {
      std::cout << "  " << node->idx_ << " @ " << node->ref_.strategy_id_
                << " -> ";
      for (auto const& edge : node->edges_) {
        std::cout << "(" << edge.to_->idx_ << " @ "
                  << edge.to_->ref_.strategy_id_ << " | " << edge.weight()
                  << " @@ " << edge.strategy_id() << ") ";
      }
      std::cout << "\n";
    }
  }
}

}  // namespace path
}  // namespace motis
