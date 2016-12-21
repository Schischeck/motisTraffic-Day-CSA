#pragma once

#include <memory>

#include "motis/path/prepare/routing/routing_strategy.h"

namespace motis {
namespace path {

struct osrm_strategy : public routing_strategy {
  osrm_strategy(strategy_id_t const, std::string path);
  ~osrm_strategy();

  std::vector<node_ref> close_nodes(geo::latlng const&) override;

  std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from,
      std::vector<node_ref> const& to) override;

  geo::polyline get_polyline(node_ref const& from,
                             node_ref const& to) const override;

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace path
}  // motis
