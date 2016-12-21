#pragma once

#include <iostream>

#include "motis/path/prepare/seq/seq_graph.h"

namespace motis {
namespace path {

inline void print_seq_path(std::vector<seq_edge const*> const& edges) {
  for (auto const& edge : edges) {
    auto const& from = *edge->from_;
    auto const& to = *edge->to_;
    std::cout << from.idx_ << " (" << from.station_idx_ << " @ "
              << from.ref_.router_id_ << ") -> " << to.idx_ << " ("
              << to.station_idx_ << " @ " << to.ref_.router_id_
              << ") = " << edge->weight() << " " << edge->router_id()
              << std::endl;
  }
}

inline void print_seq_graph(seq_graph const& g) {
  std::cout << "\n\n";
  for (auto i = 0u; i < g.station_to_nodes_.size(); ++i) {
    std::cout << "station: " << i << "\n";
    for (auto const& node : g.station_to_nodes_[i]) {
      std::cout << "  " << node->idx_ << " @ " << node->ref_.router_id_
                << " -> ";
      for (auto const& edge : node->edges_) {
        std::cout << "(" << edge.to_->idx_ << ": " << edge.weight() << " @ "
                  << edge.router_id() << ") ";
      }
      std::cout << "\n";
    }
  }
}

}  // namespace path
}  // namespace motis
