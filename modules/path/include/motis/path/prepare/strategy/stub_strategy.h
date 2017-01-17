#pragma once

#include <vector>

#include "geo/latlng.h"
#include "geo/polyline.h"

#include "utl/to_vec.h"

#include "motis/path/prepare/strategy/routing_strategy.h"

namespace motis {
namespace path {

struct stub_strategy : public routing_strategy {
  explicit stub_strategy(strategy_id_t const strategy_id,
                         std::vector<station> const& stations)
      : routing_strategy(strategy_id) {
    stations_to_coords_.set_empty_key("");
    auto id = 0u;
    for (auto const& station : stations) {
      stations_to_coords_[station.id_] = {strategy_id, id++, station.pos_};
    }
  }

  ~stub_strategy() = default;

  std::vector<node_ref> close_nodes(
      std::string const& station_id) const override {
    auto const it = stations_to_coords_.find(station_id);
    verify(it != end(stations_to_coords_), "stub: unknown station!");
    return {it->second};
  }

  bool can_route(node_ref const&) const override { return true; }

  std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from,
      std::vector<node_ref> const& to) const override {
    source_spec s{0, source_spec::category::UNKNOWN,
                  source_spec::type::STUB_ROUTE};

    return utl::to_vec(from, [&](auto const& f) {
      return utl::to_vec(to, [&](auto const& t) -> routing_result {
        return {strategy_id(), s, distance(f.coords_, t.coords_) * 5};
      });
    });
  }

  geo::polyline get_polyline(node_ref const& from,
                             node_ref const& to) const override {
    return {from.coords_, to.coords_};
  }

  hash_map<std::string, node_ref> stations_to_coords_;
};

}  // namespace path
}  // namespace motis
