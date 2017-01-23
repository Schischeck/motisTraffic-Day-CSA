#include "motis/path/prepare/rail/rail_graph_builder.h"

#include <algorithm>
#include <map>

#include "parser/util.h"

#include "utl/concat.h"
#include "utl/equal_ranges.h"
#include "utl/erase_duplicates.h"
#include "utl/repeat_n.h"
#include "utl/to_vec.h"

#include "motis/core/common/hash_map.h"
#include "motis/core/common/logging.h"

#include "motis/path/prepare/rail/rail_phantom.h"

using namespace motis::logging;
using namespace geo;

namespace motis {
namespace path {

constexpr auto kMatchRadius = 200;

using osm_idx_t = int64_t;
using node_idx_t = size_t;
using station_idx_t = size_t;
using offset_t = size_t;

rail_graph build_rail_graph(std::vector<rail_way> const& rail_ways,
                            std::vector<station> const& stations) {
  rail_graph graph;

  hash_map<osm_idx_t, std::vector<station_idx_t>> nodes_to_stations;
  nodes_to_stations.set_empty_key(std::numeric_limits<double>::max());
  auto ways_to_stations = utl::repeat_n(
      std::vector<std::pair<offset_t, station_idx_t>>{}, rail_ways.size());
  {
    rail_phantom_index phantom_idx{rail_ways};
    for (auto i = 0u; i < stations.size(); ++i) {
      auto const pair =
          phantom_idx.get_rail_phantoms(stations[i].pos_, kMatchRadius);
      for (auto const* node_p : pair.first) {
        nodes_to_stations[node_p->id_].push_back(i);
      }
      for (auto const* edge_p : pair.second) {
        ways_to_stations[edge_p->way_idx_].emplace_back(edge_p->offset_, i);
      }
    }
  }

  auto stations_to_nodes =
      utl::repeat_n(std::vector<node_idx_t>{}, stations.size());

  hash_map<osm_idx_t, rail_node*> node_map;
  node_map.set_empty_key(std::numeric_limits<osm_idx_t>::max());

  auto const make_osm_node = [&](osm_idx_t const id, geo::latlng const& pos) {
    return map_get_or_create(node_map, id, [&] {
      auto const node_idx = graph.nodes_.size();

      auto it = nodes_to_stations.find(id);
      if (it != end(nodes_to_stations)) {
        for (auto const& idx : it->second) {
          stations_to_nodes[idx].push_back(node_idx);
        }
      }

      graph.nodes_.emplace_back(std::make_unique<rail_node>(node_idx, pos));
      return graph.nodes_.back().get();
    });
  };

  auto const make_polyline = [&graph](auto const& way, size_t const from,
                                      size_t const to) {
    graph.polylines_.emplace_back(begin(way.polyline_) + from,
                                  begin(way.polyline_) + to + 1);
    return graph.polylines_.size() - 1;
  };

  auto const make_way_node = [&](auto const& lb, auto const& ub,
                                 geo::latlng const& pos) {
    auto const node_idx = graph.nodes_.size();

    for (auto it = lb; it != ub; ++it) {
      stations_to_nodes[it->second].push_back(node_idx);
    }

    graph.nodes_.emplace_back(std::make_unique<rail_node>(node_idx, pos));
    return graph.nodes_.back().get();
  };

  auto const make_edges = [&graph](auto from, auto to,
                                   auto const polyline_idx) {
    auto const dist = geo::length(graph.polylines_[polyline_idx]);

    from->edges_.emplace_back(polyline_idx, true, dist, from, to);
    to->edges_.emplace_back(polyline_idx, false, dist, to, from);
  };

  for (auto i = 0u; i < rail_ways.size(); ++i) {
    auto const& way = rail_ways[i];

    auto prev_node = make_osm_node(way.from_, way.polyline_.front());
    auto prev_offset = 0;

    utl::equal_ranges(
        ways_to_stations[i],
        [](auto const& lhs, auto const& rhs) { return lhs < rhs; },
        [&](auto const lb, auto const ub) {
          auto const polyline_idx = make_polyline(way, prev_offset, lb->first);
          auto curr_node =
              make_way_node(lb, ub, graph.polylines_.back().back());

          make_edges(prev_node, curr_node, polyline_idx);

          prev_node = curr_node;
          prev_offset = lb->first;
        });

    auto const polyline_idx =
        make_polyline(way, prev_offset, way.polyline_.size() - 1);
    auto curr_node = make_osm_node(way.to_, way.polyline_.back());

    make_edges(prev_node, curr_node, polyline_idx);
  }

  for (auto i = 0u; i < stations.size(); ++i) {
    graph.stations_to_nodes_[stations[i].id_] = stations_to_nodes[i];
  }

  return graph;
}

}  // namespace path
}  // namespace motis
