#pragma once

#include "geo/point_rtree.h"

#include "utl/concat.h"
#include "utl/to_vec.h"

#include "parser/util.h"

#include "motis/core/common/hash_map.h"

#include "motis/path/prepare/rail/rail_graph.h"
#include "motis/path/prepare/rail/rail_graph_builder.h"
#include "motis/path/prepare/rail/rail_router.h"

#include "motis/path/prepare/strategy/routing_strategy.h"

namespace motis {
namespace path {

struct rail_strategy : public routing_strategy {
  rail_strategy(strategy_id_t const id, std::vector<station> const& stations,
                std::vector<rail_way> const& rail_ways)
      : routing_strategy(id), graph_(build_rail_graph(rail_ways, stations)) {
    print_rail_graph_stats(graph_);
  }

  ~rail_strategy() = default;

  std::vector<node_ref> close_nodes(
      std::string const& station_id) const override {
    auto const it = graph_.stations_to_nodes_.find(station_id);
    verify(it != end(graph_.stations_to_nodes_), "rail: unknown station!");

    return utl::to_vec(it->second, [&](auto const& idx) -> node_ref {
      auto const& edge = graph_.nodes_[idx]->edges_.front();
      auto const& polyline = graph_.polylines_[edge.polyline_idx_];

      auto const& pos = edge.is_forward() ? polyline.front() : polyline.back();

      return {strategy_id(), idx, pos};
    });
  }

  bool is_cacheable() const override { return true; }

  bool can_route(node_ref const& ref) const override {
    return ref.strategy_id() == strategy_id();
  }

  routing_result_matrix find_routes(
      std::vector<node_ref> const& from,
      std::vector<node_ref> const& to) const override {
    auto const from_ids = utl::to_vec(from, [](auto&& f) { return f.id_; });
    auto const to_ids = utl::to_vec(to, [](auto&& t) { return t.id_; });

    auto const path_to_result = [&](auto const& path) {
      if (path.empty()) {
        return routing_result{};
      }
      auto cost = 0.;
      for (auto const& edge : path) {
        verify(edge != nullptr, "rail (find_routes) found invalid edge");
        cost += edge->dist_;
      }
      source_spec s{0, source_spec::category::RAILWAY,
                    source_spec::type::RAIL_ROUTE};
      return routing_result{strategy_id(), s, cost};
    };

    auto const route = [&path_to_result](auto const& froms, auto const& tos) {
      return utl::to_vec(froms, [&](auto const& f) {
        return utl::to_vec(shortest_paths(graph_, f, tos), path_to_result);
      });
    };

    if (from.size() <= to.size()) {
      return routing_result_matrix{route(from, to)};
    } else {
      return routing_result_matrix{route(to, from), true};
    }
  }

  geo::polyline get_polyline(node_ref const& from,
                             node_ref const& to) const override {
    verify(from.strategy_id_ == strategy_id(), "rail bad 'from' strategy_id");
    verify(to.strategy_id_ == strategy_id(), "rail bad 'to' strategy_id");

    auto const path = shortest_paths(graph_, {from.id_}, {to.id_})[0];
    geo::polyline result;
    for (auto const& edge : path) {
      verify(edge != nullptr, "rail (get_polyline) found invalid edge");
      auto const& polyline = graph_.polylines_[edge->polyline_idx_];
      if (edge->is_forward()) {
        utl::concat(result, polyline);
      } else {
        auto reversed{polyline};  // make this faster
        std::reverse(begin(reversed), end(reversed));
        utl::concat(result, reversed);
      }
    }
    return result;
  }

  rail_graph graph_;
};

}  // namespace path
}  // namespace motis
