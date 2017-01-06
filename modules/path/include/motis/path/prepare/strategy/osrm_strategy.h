#pragma once

#include <memory>

#include "motis/path/prepare/strategy/routing_strategy.h"
#include "motis/path/prepare/schedule/stations.h"

namespace motis {
namespace path {

struct osrm_strategy : public routing_strategy {
  osrm_strategy(strategy_id_t const, std::vector<station> const&,
                std::string const& osrm_path);
  ~osrm_strategy();

  std::vector<node_ref> close_nodes(
      std::string const& station_id) const override;

  bool can_route(node_ref const&) const override;

  std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from,
      std::vector<node_ref> const& to) const override;

  geo::polyline get_polyline(node_ref const& from,
                             node_ref const& to) const override;

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace path
}  // motis
