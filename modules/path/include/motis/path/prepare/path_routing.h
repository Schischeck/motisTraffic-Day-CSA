#pragma once

#include <memory>

#include "motis/path/prepare/rel/polyline_aggregator.h"
#include "motis/path/prepare/strategy/routing_strategy.h"

namespace motis {
namespace path {

struct path_routing {
  path_routing();
  ~path_routing();

  path_routing(path_routing&&);
  path_routing& operator=(path_routing&&);

  std::vector<routing_strategy*> strategies_for(source_spec::category const);

  struct strategies;
  std::unique_ptr<strategies> strategies_;
};

path_routing make_path_routing(std::string const& osm_path,
                               std::string const& osrm_path);

}  // namespace path
}  // namespace motis