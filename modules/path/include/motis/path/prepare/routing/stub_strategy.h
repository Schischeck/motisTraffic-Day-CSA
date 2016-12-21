#pragma once

#include <vector>

#include "geo/latlng.h"
#include "geo/polyline.h"

#include "utl/to_vec.h"

#include "motis/path/prepare/routing/routing_strategy.h"

namespace motis {
namespace path {

struct stub_strategy : public routing_strategy {
  explicit stub_strategy(strategy_id_t const strategy_id)
      : routing_strategy(strategy_id) {}

  ~stub_strategy() = default;

  std::vector<node_ref> close_nodes(geo::latlng const& pos) override {
    return {{pos, {}, strategy_id()}};
  }

  std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from,
      std::vector<node_ref> const& to) override {
    source_spec s{0, source_spec::category::UNKNOWN,
                  source_spec::type::STUB_ROUTE};

    return utl::to_vec(from, [&](auto const& f) {
      return utl::to_vec(to, [&](auto const& t) -> routing_result {


        return {s, strategy_id(), distance(f.coords_, t.coords_) * 5};
      });
    });
  }

  geo::polyline get_polyline(node_ref const& from,
                             node_ref const& to) const override {
    return {from.coords_, to.coords_};
  }
};

}  // namespace path
}  // motis
