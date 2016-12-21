#pragma once

#include <memory>

#include "motis/path/prepare/routing/routing_strategy.h"

namespace motis {
namespace path {

struct osrm_osrm : public routing_strategy {

  explicit osrm_osrm(std::size_t router_id, std::string path);
  ~osrm_osrm();

  std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from, std::vector<node_ref> const& to);

  std::vector<node_ref> close_nodes(geo::latlng const& latlng);

  geo::polyline get_polyline(node_ref const& from, node_ref const& to);

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace path
}  // motis
