#pragma once

#include "geo/point_rtree.h"

#include "utl/concat.h"
#include "utl/to_vec.h"

#include "parser/util.h"

#include "motis/path/prepare/rail/rail_graph.h"
#include "motis/path/prepare/rail/rail_router.h"

#include "motis/path/prepare/strategy/routing_strategy.h"

namespace motis {
namespace path {

struct rail_strategy : public routing_strategy {
  static constexpr auto kMatchRadius = 200;

  explicit rail_strategy(strategy_id_t const id, rail_graph graph)
      : routing_strategy(id), graph_(std::move(graph)) {
    for (auto i = 0u; i < graph_.nodes_.size(); ++i) {
      refs_.emplace_back(graph_.nodes_[i]->pos_, node_ref_id{i}, strategy_id());
    }

    rtree_ = geo::make_point_rtree(refs_, [](auto&& r) { return r.coords_; });
  }

  ~rail_strategy() = default;

  std::vector<node_ref> close_nodes(geo::latlng const& latlng) override {
    return utl::to_vec(rtree_.in_radius(latlng, kMatchRadius),
                       [&](auto const& idx) { return refs_[idx]; });
  }

  bool can_route(node_ref const& ref) const override {
    return ref.strategy_id() == strategy_id();
  }

  std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from,
      std::vector<node_ref> const& to) override {
    auto const to_ids = utl::to_vec(to, [](auto&& t) { return t.id_.id_; });
    return utl::to_vec(from, [&](auto const& f) {
      return utl::to_vec(
          shortest_paths(graph_, {f.id_.id_}, to_ids), [&](auto const& path) {
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
            return routing_result{s, strategy_id(), cost};
          });
    });
  }

  geo::polyline get_polyline(node_ref const& from,
                             node_ref const& to) const override {
    verify(from.strategy_id_ == strategy_id(), "rail bad 'from' strategy_id");
    verify(to.strategy_id_ == strategy_id(), "rail bad 'to' strategy_id");

    auto const path = shortest_paths(graph_, {from.id_.id_}, {to.id_.id_})[0];

    geo::polyline result;
    for (auto const& edge : path) {
      verify(edge != nullptr, "rail (get_polyline) found invalid edge");

      utl::concat(result, edge->polyline_);
    }
    return result;
  }

  rail_graph graph_;
  std::vector<node_ref> refs_;
  geo::point_rtree rtree_;
};

}  // namespace path
}  // motis
