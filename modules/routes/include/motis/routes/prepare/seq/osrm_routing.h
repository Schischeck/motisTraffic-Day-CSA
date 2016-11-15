#pragma once

#include <memory>

#include "motis/routes/prepare/seq/routing_strategy.h"

namespace motis {
namespace routes {

struct osrm_routing : public routing_strategy {

  osrm_routing(std::string path);
  ~osrm_routing();

  std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from, std::vector<node_ref> const& to);

  std::vector<node_ref> close_nodes(std::string const& station_id);

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // routes
}  // motis
