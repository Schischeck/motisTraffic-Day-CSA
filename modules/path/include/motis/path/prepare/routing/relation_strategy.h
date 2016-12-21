#pragma once

#include <set>

#include "geo/point_rtree.h"
#include "geo/polyline.h"

#include "parser/util.h"

#include "utl/repeat_n.h"
#include "utl/to_vec.h"

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
    std::set<size_t> visited;
    for (auto const& idx : rtree_.in_radius(latlng, kMatchRadius)) {
      auto const& ref = refs_[idx];

      if (visited.find(ref.id_.relation_id_) != end(visited)) {
        continue;
      }

      visited.insert(ref.id_.relation_id_);
      result.push_back(ref);
    }

    return result;
  }

  std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from,
      std::vector<node_ref> const& to) override {
    return utl::to_vec(from, [&](auto const& f) -> std::vector<routing_result> {
      if (f.strategy_id() != strategy_id()) {
        return utl::repeat_n(routing_result{}, to.size());
      }

      return utl::to_vec(to, [&](auto const& t) -> routing_result {
        if (t.strategy_id() != strategy_id() ||
            f.id_.relation_id_ != t.id_.relation_id_) {
          return {};
        }

        auto const p = polylines_[f.id_.relation_id_];
        return {p.source_, strategy_id(), 0};
      });
    });
  }

  geo::polyline get_polyline(node_ref const& from,
                             node_ref const& to) const override {
    verify(from.strategy_id_ == strategy_id(), "rel: bad 'from' strategy_id");
    verify(to.strategy_id_ == strategy_id(), "rel: bad 'to' strategy_id");
    verify(from.id_.relation_id_ == to.id_.relation_id_, "rel: id mismatch");

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
