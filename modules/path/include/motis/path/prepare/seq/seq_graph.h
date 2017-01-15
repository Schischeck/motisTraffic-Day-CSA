#pragma once

#include <limits>
#include <memory>
#include <vector>

#include "geo/latlng.h"
#include "geo/polyline.h"

#include "motis/path/prepare/source_spec.h"
#include "motis/path/prepare/strategy/routing_strategy.h"

namespace motis {
namespace path {

struct seq_edge;

struct seq_node {
  seq_node(size_t const idx, size_t const station_idx, node_ref const& ref)
      : idx_(idx),
        incomming_edges_count_(0),
        station_idx_(station_idx),
        ref_(ref) {}

  size_t strategy_id() const { return ref_.strategy_id_; }

  size_t idx_;

  std::vector<seq_edge> edges_;
  size_t incomming_edges_count_;

  size_t station_idx_;
  node_ref ref_;
};

struct seq_edge {
  seq_edge(seq_node* from, seq_node* to, routing_result routing)
      : from_(from), to_(to), routing_(std::move(routing)) {}

  double weight() const { return routing_.weight_; }
  size_t strategy_id() const { return routing_.strategy_id_; }
  source_spec source_spec() const { return routing_.source_; }

  seq_node* from_;
  seq_node* to_;
  routing_result routing_;
};

struct seq_graph {
  explicit seq_graph(size_t const seq_size) : seq_size_(seq_size) {}

  size_t seq_size_;
  std::vector<std::unique_ptr<seq_node>> nodes_;

  std::vector<std::size_t> initials_;
  std::vector<std::size_t> goals_;
};

}  // namespace path
}  // namespace motis
