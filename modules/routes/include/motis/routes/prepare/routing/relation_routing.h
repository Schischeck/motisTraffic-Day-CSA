#pragma once

#include <memory>

#include "motis/routes/prepare/rel/polyline_aggregator.h"
#include "motis/routes/prepare/routing/routing_strategy.h"

namespace motis {
namespace routes {

struct relation_routing : public routing_strategy {

  explicit relation_routing(std::size_t router_idm,
                            std::vector<aggregated_polyline> const& polylines);
  ~relation_routing();

  std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from, std::vector<node_ref> const& to);

  std::vector<node_ref> close_nodes(geo::latlng const& latlng);

  geo::polyline get_polyline(node_ref const& from, node_ref const& to);

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // routes
}  // motis
