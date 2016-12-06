#pragma once

#include <memory>

#include "motis/routes/prepare/seq/routing_strategy.h"

namespace motis {
namespace routes {

struct rail_routing : public routing_strategy {

  explicit rail_routing(std::size_t router_id, std::string path);
  ~rail_routing();

  std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from, std::vector<node_ref> const& to);

  std::vector<node_ref> close_nodes(node_ref const& station);

  geo::polyline get_polyline(node_ref const& from, node_ref const& to);

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // routes
}  // motis
