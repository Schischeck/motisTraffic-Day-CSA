#include "motis/path/prepare/routing/path_routing.h"

#include "motis/path/prepare/routing/osrm_strategy.h"
#include "motis/path/prepare/routing/relation_strategy.h"
#include "motis/path/prepare/routing/stub_strategy.h"

namespace motis {
namespace path {

struct path_routing::strategies {
  std::unique_ptr<osrm_strategy> osrm_strategy_;
  std::unique_ptr<relation_strategy> relation_strategy_;
  std::unique_ptr<stub_strategy> stub_strategy_;
};

using strategies_ptr = std::unique_ptr<path_routing::strategies>;

path_routing::path_routing()
    : strategies_(std::make_unique<path_routing::strategies>()) {}
path_routing::~path_routing() = default;

path_routing::path_routing(path_routing&&) = default;
path_routing& path_routing::operator=(path_routing&&) = default;

std::vector<routing_strategy*> path_routing::strategies_for(
    source_spec::category const cat) {
  std::vector<routing_strategy*> result;

  // always relations first
  result.push_back(strategies_->relation_strategy_.get());

  if (cat == source_spec::category::BUS) {
    result.push_back(strategies_->osrm_strategy_.get());
  }

  // always stub last
  result.push_back(strategies_->stub_strategy_.get());

  return result;
}

path_routing make_path_routing(
    std::vector<aggregated_polyline> const& polylines_from_rels,
    std::string const& osrm_path) {
  path_routing r;

  strategy_id_t id = 0;

  r.strategies_->osrm_strategy_ =
      std::make_unique<osrm_strategy>(id++, osrm_path);
  r.strategies_->relation_strategy_ =
      std::make_unique<relation_strategy>(id++, polylines_from_rels);
  r.strategies_->stub_strategy_ = std::make_unique<stub_strategy>(id++);

  return r;
}

}  // namespace path
}  // namespace motis