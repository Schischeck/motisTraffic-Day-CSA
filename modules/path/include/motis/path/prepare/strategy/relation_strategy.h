#pragma once

#include <set>

#include "geo/point_rtree.h"
#include "geo/polyline.h"

#include "parser/util.h"

#include "utl/repeat_n.h"
#include "utl/to_vec.h"

#include "motis/core/common/hash_map.h"

#include "motis/path/prepare/rel/polyline_aggregator.h"
#include "motis/path/prepare/schedule/stations.h"
#include "motis/path/prepare/strategy/routing_strategy.h"

namespace motis {
namespace path {

static constexpr auto kRelationMatchRadius = 200;

struct relation_strategy : public routing_strategy {
  struct relation_node_ref {
    relation_node_ref(size_t polyline_idx, size_t point_idx, geo::latlng pos)
        : polyline_idx_(polyline_idx),
          point_idx_(point_idx),
          pos_(std::move(pos)) {}

    size_t polyline_idx_;  // n-th polyline
    size_t point_idx_;  // m-th point on polyline

    geo::latlng pos_;
  };

  relation_strategy(strategy_id_t const id,
                    std::vector<station> const& stations,
                    std::vector<aggregated_polyline> polylines,
                    source_spec::category category)
      : routing_strategy(id), polylines_(std::move(polylines)) {
    for (auto i = 0u; i < polylines_.size(); ++i) {
      if (polylines_[i].source_.category_ != category &&
          polylines_[i].source_.category_ != source_spec::category::UNKNOWN) {
        continue;
      }
      auto const& polyline = polylines_[i].polyline_;
      for (auto j = 0u; j < polyline.size(); ++j) {
        node_refs_.emplace_back(i, j, polyline[j]);
      }
    }

    auto const rtree =
        geo::make_point_rtree(node_refs_, [](auto const& r) { return r.pos_; });

    stations_to_refs_.set_empty_key("");
    for (auto const& station : stations) {
      std::set<size_t> relations;
      std::vector<size_t> node_refs;
      for (auto id : rtree.in_radius(station.pos_, kRelationMatchRadius)) {
        if (relations.find(node_refs_[id].polyline_idx_) != end(relations)) {
          continue;
        }
        relations.insert(node_refs_[id].polyline_idx_);
        node_refs.push_back(id);
      }
      stations_to_refs_[station.id_] = std::move(node_refs);
    }
  }

  ~relation_strategy() override = default;

  std::vector<node_ref> close_nodes(
      std::string const& station_id) const override {
    auto const it = stations_to_refs_.find(station_id);
    verify(it != end(stations_to_refs_), "relations: unknown station!");

    return utl::to_vec(it->second, [&, this](auto const& idx) -> node_ref {
      return {this->strategy_id(), idx, node_refs_[idx].pos_};
    });
  }

  bool is_cacheable() const override { return false; }

  bool can_route(node_ref const& ref) const override {
    return ref.strategy_id() == strategy_id();
  }

  routing_result_matrix find_routes(
      std::vector<node_ref> const& from,
      std::vector<node_ref> const& to) const override {
    return routing_result_matrix{utl::to_vec(from, [&](auto const& f) {
      auto const& f_ref = node_refs_[f.id_];

      return utl::to_vec(to, [&, this](auto const& t) -> routing_result {
        auto const& t_ref = node_refs_[t.id_];

        if (f_ref.polyline_idx_ != t_ref.polyline_idx_) {
          return {};
        }

        auto const p = polylines_[f_ref.polyline_idx_];
        return {this->strategy_id(), p.source_, 0};
      });
    })};
  }

  geo::polyline get_polyline(node_ref const& from,
                             node_ref const& to) const override {
    verify(from.strategy_id_ == strategy_id(), "rel: bad 'from' strategy_id");
    verify(to.strategy_id_ == strategy_id(), "rel: bad 'to' strategy_id");

    auto const& f_ref = node_refs_[from.id_];
    auto const& t_ref = node_refs_[to.id_];
    verify(f_ref.polyline_idx_ == t_ref.polyline_idx_, "rel: id mismatch");

    // XXX direction !?
    auto p = polylines_[f_ref.polyline_idx_].polyline_;
    return geo::extract(p, f_ref.point_idx_, t_ref.point_idx_);
  }

  std::vector<aggregated_polyline> polylines_;

  std::vector<relation_node_ref> node_refs_;
  hash_map<std::string, std::vector<size_t>> stations_to_refs_;
};

}  // namespace path
}  // namespace motis
