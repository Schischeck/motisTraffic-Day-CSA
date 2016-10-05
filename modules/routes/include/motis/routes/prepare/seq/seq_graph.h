#pragma once

#include <memory>
#include <vector>

#include "geo/latlng.h"
#include "geo/polyline.h"

namespace motis {
namespace routes {

struct seq_edge;

struct seq_node {
  seq_node(size_t idx, size_t station_idx, node_ref const& ref)
      : idx_(idx), station_idx_(station_idx), ref_(ref) {}

  size_t idx_;
  std::vector<seq_edge> edges_;

  size_t station_idx_;
  node_ref ref_;
};

struct seq_edge {
  seq_edge(seq_node* from, seq_node* to, geo::polyline p, double weight)
      : from_(from), to_(to), p_(std::move(p)), weight_(weight) {}

  seq_node* from_;
  seq_node* to_;
  geo::polyline p_;
  double weight_;
};

struct seq_graph {
  seq_graph(size_t seq_length) { station_to_nodes_.resize(seq_length); }

  std::vector<std::vector<seq_node*>> station_to_nodes_;
  std::vector<std::unique_ptr<seq_node>> nodes_;

  std::vector<std::size_t> initials_;
  std::vector<std::size_t> goals_;
};

// inline void dump_seq_graph(seq_graph const& g) {
//   std::cout << "\n\n";
//   for(auto i = 0u; i < g.station_to_nodes_.size(); ++i) {
//     std::cout << i << "\n";
//     for(auto const& node : g.station_to_nodes_[i]) {
//       std::cout << "  " << node->idx_ << " -> ";
//       for(auto const& edge : node->edges_) {
//         std::cout << "(" << edge.to_->idx_ << ": " << edge.weight_ << ") ";
//       }
//       std::cout << "\n";
//     }
//   }
// }


}  // namespace routes
}  // namespace motis
