#pragma once

#include "geo/point_rtree.h"
#include "geo/polyline.h"

#include "motis/path/prepare/rel/polyline_aggregator.h"
#include "motis/path/prepare/routing/routing_strategy.h"

namespace motis {
namespace path {

struct relation_strategy : public routing_strategy {

  static constexpr auto kMatchRadius = 200;

  relation_strategy(strategy_id_t const id,
                    std::vector<aggregated_polyline> const& polylines)
      : routing_strategy(id), polylines_(polylines) {
    for (auto i = 0u; i < polylines_.size(); ++i) {
      auto const& polyline = polylines_[i].polyline_;
      for (auto j = 0u; j < polyline.size(); ++j) {
        refs_.emplace_back(polyline[j], node_ref_id(i, j), strategy_id());
      }
    }
    rtree_ = geo::make_point_rtree(refs_, [](auto&& r) { return r.coords_; });
  }

  ~relation_strategy() = default;

  std::vector<node_ref> close_nodes(geo::latlng const& latlng) override {
    std::vector<node_ref> result;
    auto nodes = rtree_.in_radius_with_distance(latlng, kMatchRadius);
    std::vector<std::size_t> visited;
    for (auto i = 0u; i < (nodes.size() < 5 ? nodes.size() : 5); ++i) {
      auto& ref = refs_[nodes[i].second];
      if (std::find(begin(visited), end(visited), ref.id_.relation_id_) !=
          end(visited)) {
        continue;
      }
      visited.push_back(ref.id_.relation_id_);
      result.push_back(refs_[nodes[i].second]);
    }
    return result;
  }

  std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from,
      std::vector<node_ref> const& to) override {
    std::vector<std::vector<routing_result>> result{from.size()};
    for (auto const& f : from) {
      if (f.router_id_ != strategy_id()) {
        result.push_back({});
        continue;
      }
      std::vector<routing_result> from_result;
      for (auto const& t : to) {
        if (t.router_id_ != strategy_id() ||
            f.id_.relation_id_ != t.id_.relation_id_) {
          continue;
        }
        auto p = polylines_[f.id_.relation_id_];
        from_result.emplace_back(p.source_, strategy_id(), 0);
      }
      result.push_back(std::move(from_result));
    }
    return result;
  }

  geo::polyline get_polyline(node_ref const& from,
                             node_ref const& to) const override {
    if (from.router_id_ != strategy_id() || to.router_id_ != strategy_id() ||
        from.id_.relation_id_ != to.id_.relation_id_) {
      return {};
    }

    // XXX direction !?
    auto p = polylines_[from.id_.relation_id_].polyline_;
    return geo::polyline{begin(p) + std::min(from.id_.id_, to.id_.id_),
                         begin(p) + std::max(from.id_.id_, to.id_.id_) + 1};
  }

  std::vector<aggregated_polyline> polylines_;
  std::vector<node_ref> refs_;
  geo::point_rtree rtree_;
};

}  // namespace path
}  // motis
