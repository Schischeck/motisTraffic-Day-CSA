#pragma once

#include "geo/point_rtree.h"

#include "motis/path/prepare/rail/rail_graph.h"

#include "motis/path/prepare/strategy/routing_strategy.h"

namespace motis {
namespace path {

struct rail_strategy : public routing_strategy {

  explicit rail_strategy(strategy_id_t const strategy_id, rail_graph graph)
      : routing_strategy(strategy_id), graph_(std::move(graph)) {


    // for (auto i = 0u; i < polylines_.size(); ++i) {
    //   auto const& polyline = polylines_[i].polyline_;
    //   for (auto j = 0u; j < polyline.size(); ++j) {
    //     refs_.emplace_back(polyline[j], node_ref_id(i, j), strategy_id());
    //   }
    // }
    // rtree_ = geo::make_point_rtree(refs_, [](auto&& r) { return r.coords_; });

      }

  ~rail_strategy() = default;

  std::vector<node_ref> close_nodes(geo::latlng const&) override;

  std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from,
      std::vector<node_ref> const& to) override;

  geo::polyline get_polyline(node_ref const& from,
                             node_ref const& to) const override;

  rail_graph graph_;
  std::vector<node_ref> refs_;
  geo::point_rtree rtree_;
};

}  // namespace path
}  // motis
