#pragma once

#include <algorithm>
#include <memory>
#include <numeric>
#include <vector>

#include "geo/latlng.h"
#include "geo/polyline.h"

#include "utl/to_vec.h"

#include "motis/core/common/logging.h"

namespace motis {
namespace path {

struct rail_node;
struct rail_edge_info;

struct rail_graph {
  std::vector<std::unique_ptr<rail_node>> nodes_;
  std::vector<std::unique_ptr<rail_edge_info>> infos_;
};

struct rail_node {
  rail_node(size_t idx, int64_t id, geo::latlng pos)
      : idx_(idx), id_(id), pos_(pos) {}
  size_t idx_;  // idx in railway_graph.nodes_

  int64_t id_;  // from osm
  geo::latlng pos_;

  std::vector<rail_edge> edges_;
};

struct rail_edge {
  rail_edge(uint64_t info_idx, bool forward, size_t dist, rail_node const* to)
      : info_idx_(info_idx), forward_(forward), dist_(dist), to_(to) {}

  uint64_t info_idx_ : 63;
  uint64_t forward_ : 1;

  size_t dist_;
  rail_node const* to_;
};

struct rail_edge_info {
  rail_edge_info(geo::polyline polyline, size_t dist, rail_node const* from,
                 rail_node const* to)
      : polyline_(std::move(polyline)), dist_(dist), nodes_(from, to) {}

  // int64_t id_;  // from osm

  size_t dist_;
  geo::polyline polyline_;

  std::pair<rail_node const*, rail_node const*> nodes_;
};

inline void print_rail_graph_stats(rail_graph const& graph) {
  namespace ml = motis::logging;
  LOG(ml::info) << "rail graph stats";
  LOG(ml::info) << "- nodes: " << graph.nodes_.size();

  auto vec =
      utl::to_vec(graph.nodes_, [](auto const& n) { return n->edge_.size(); });
  auto const count = std::accumulate(begin(vec), end(vec), 0);
  auto const avg = count / vec.size();

  LOG(ml::info) << "- edge: " << count;
  LOG(ml::info) << "- degree: "  //
                << " 0:" << std::count(begin(vec), end(vec), 0)
                << " 1:" << std::count(begin(vec), end(vec), 1)
                << " 2:" << std::count(begin(vec), end(vec), 2);

  std::sort(begin(vec), end(vec));
  LOG(ml::info) << "- degree: "  //
                << " avg:" << avg  //
                << " q75:" << vec[0.75 * (vec.size() - 1)]  //
                << " q90:" << vec[0.90 * (vec.size() - 1)]  //
                << " q95:" << vec[0.95 * (vec.size() - 1)];

  size_t coord_count = 0;
  for (auto const& node : graph.nodes_) {
    for (auto const& link : node->edge_) {
      coord_count += link.polyline_.size();
    }
  }
  LOG(ml::info) << "- coordinates (approx): " << (coord_count / 2);
}

}  // namespace path
}  // namespace motis
