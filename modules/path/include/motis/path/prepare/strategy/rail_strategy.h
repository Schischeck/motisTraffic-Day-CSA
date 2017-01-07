#pragma once

#include "geo/point_rtree.h"

#include "utl/concat.h"
#include "utl/to_vec.h"

#include "parser/util.h"

#include "motis/core/common/hash_map.h"

#include "motis/path/prepare/rail/rail_graph.h"
#include "motis/path/prepare/rail/rail_phantom.h"
#include "motis/path/prepare/rail/rail_router.h"

#include "motis/path/prepare/strategy/routing_strategy.h"

namespace motis {
namespace path {

struct rail_strategy : public routing_strategy {
  static constexpr auto kMatchRadius = 200;

  explicit rail_strategy(strategy_id_t const id,
                         std::vector<station> const& stations, rail_graph graph)
      : routing_strategy(id), graph_(std::move(graph)) {
    rail_phantom_index phantom_index{graph_};

    stations_to_phantoms_.set_empty_key("");
    for (auto const& station : stations) {
      stations_to_phantoms_[station.id_] = utl::to_vec(
          phantom_index.get_rail_phantoms(station.pos_, kMatchRadius),
          [&](auto const& phantom) {
            rail_phantoms_.push_back(phantom);
            return rail_phantoms_.size() - 1;
          });
    }
  }

  ~rail_strategy() = default;

  std::vector<node_ref> close_nodes(
      std::string const& station_id) const override {
    auto const it = stations_to_phantoms_.find(station_id);
    verify(it != end(stations_to_phantoms_), "rail: unknown station!");

    return utl::to_vec(it->second, [&](auto const& idx) -> node_ref {
      return {strategy_id(), idx, rail_phantoms_[idx].pos_};
    });
  }

  bool can_route(node_ref const& ref) const override {
    return ref.strategy_id() == strategy_id();
  }

  std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from,
      std::vector<node_ref> const& to) const override {
    auto const to_phantoms =
        utl::to_vec(to, [&](auto const& t) { return rail_phantoms_[t.id_]; });
    return utl::to_vec(from, [&](auto const& f) {
      auto const& f_phantom = rail_phantoms_[f.id_];
      return utl::to_vec(shortest_paths(graph_, f_phantom, to_phantoms),
                         [&](auto const& path) {
                           if (!path.valid_) {
                             return routing_result{};
                           }

                           auto const cost = length(graph_, path);
                           source_spec s{0, source_spec::category::RAILWAY,
                                         source_spec::type::RAIL_ROUTE};
                           return routing_result{strategy_id(), s, cost};
                         });
    });
  }

  geo::polyline get_polyline(node_ref const& from,
                             node_ref const& to) const override {
    verify(from.strategy_id_ == strategy_id(), "rail bad 'from' strategy_id");
    verify(to.strategy_id_ == strategy_id(), "rail bad 'to' strategy_id");

    auto const path = shortest_paths(graph_, rail_phantoms_[from.id_],
                                     {rail_phantoms_[to.id_]})[0];
    return to_polyline(graph_, path);
  }

  rail_graph graph_;

  std::vector<rail_phantom> rail_phantoms_;
  hash_map<std::string, std::vector<size_t>> stations_to_phantoms_;
};

}  // namespace path
}  // motis
