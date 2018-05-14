#pragma once

#include <algorithm>
#include <memory>
#include <numeric>
#include <vector>

#include "geo/latlng.h"
#include "geo/polyline.h"

#include "utl/concat.h"
#include "utl/erase_duplicates.h"
#include "utl/to_vec.h"

#include "motis/core/common/hash_map.h"
#include "motis/core/common/logging.h"

namespace motis {
namespace path {

struct rail_node;
struct rail_edge;

struct rail_graph {
  rail_graph() { stations_to_nodes_.set_empty_key(""); }

  std::vector<std::unique_ptr<rail_node>> nodes_;
  std::vector<geo::polyline> polylines_;
  hash_map<std::string, std::vector<size_t>> stations_to_nodes_;
};

struct rail_node {
  rail_node(size_t const idx, geo::latlng pos) : idx_(idx), pos_(pos) {}

  size_t idx_;
  geo::latlng pos_;
  std::vector<rail_edge> edges_;
};

struct rail_edge {
  rail_edge(uint64_t polyline_idx, bool forward, size_t dist,
            rail_node const* from, rail_node const* to)
      : polyline_idx_(polyline_idx),
        forward_(forward ? 1u : 0u),
        dist_(dist),
        from_(from),
        to_(to) {}

  bool is_forward() const { return forward_; }

  uint64_t polyline_idx_ : 63;
  uint64_t forward_ : 1;
  size_t dist_;

  rail_node const* from_;
  rail_node const* to_;
};

inline void print_rail_graph_stats(rail_graph const& graph) {
  namespace ml = motis::logging;
  LOG(ml::info) << "rail graph stats";
  LOG(ml::info) << "- nodes: " << graph.nodes_.size();

  auto vec =
      utl::to_vec(graph.nodes_, [](auto const& n) { return n->edges_.size(); });
  auto const count = std::accumulate(begin(vec), end(vec), 0);
  auto const avg = count / vec.size();

  LOG(ml::info) << "- edges: " << count;
  LOG(ml::info) << "- degree: "  //
                << " 0:" << std::count(begin(vec), end(vec), 0u)
                << " 1:" << std::count(begin(vec), end(vec), 1u)
                << " 2:" << std::count(begin(vec), end(vec), 2u);

  std::sort(begin(vec), end(vec));
  LOG(ml::info) << "- degree: "  //
                << " avg:" << avg  //
                << " q75:" << vec[0.75 * (vec.size() - 1)]  //
                << " q90:" << vec[0.90 * (vec.size() - 1)]  //
                << " q95:" << vec[0.95 * (vec.size() - 1)];

  size_t coord_count = 0;
  for (auto const& polyline : graph.polylines_) {
    coord_count += polyline.size();
  }
  LOG(ml::info) << "- coordinates (on edges): " << (coord_count / 2);

  size_t mapped_stations = 0;
  std::vector<size_t> station_nodes;
  for (auto const& pair : graph.stations_to_nodes_) {
    if (!pair.second.empty()) {
      ++mapped_stations;
    }
    utl::concat(station_nodes, pair.second);
  }
  utl::erase_duplicates(station_nodes);

  LOG(ml::info) << "- mapped stations: " << mapped_stations;
  LOG(ml::info) << "- unique nodes for stations: " << station_nodes.size();
}

}  // namespace path
}  // namespace motis
