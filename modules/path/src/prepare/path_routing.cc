#include "motis/path/prepare/path_routing.h"

#include "boost/filesystem.hpp"

#include "motis/core/common/logging.h"

#include "motis/path/prepare/rail/rail_cache.h"
#include "motis/path/prepare/rail/rail_graph_builder.h"
#include "motis/path/prepare/rail/rail_way.h"
#include "motis/path/prepare/rel/polyline_cache.h"

#include "motis/path/prepare/strategy/osrm_strategy.h"
#include "motis/path/prepare/strategy/rail_strategy.h"
#include "motis/path/prepare/strategy/relation_strategy.h"
#include "motis/path/prepare/strategy/stub_strategy.h"

namespace fs = boost::filesystem;

namespace motis {
namespace path {

struct path_routing::strategies {
  std::unique_ptr<osrm_strategy> osrm_strategy_;
  std::unique_ptr<rail_strategy> rail_strategy_;
  std::unique_ptr<rail_strategy> sub_strategy_;
  std::unique_ptr<relation_strategy> relation_rail_strategy_;
  std::unique_ptr<relation_strategy> relation_bus_strategy_;
  std::unique_ptr<relation_strategy> relation_sub_strategy_;
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

  if (cat == source_spec::category::RAILWAY) {
    result.push_back(strategies_->relation_rail_strategy_.get());
    result.push_back(strategies_->rail_strategy_.get());
  }

  if (cat == source_spec::category::BUS) {
    result.push_back(strategies_->relation_bus_strategy_.get());
    result.push_back(strategies_->osrm_strategy_.get());
  }

  if (cat == source_spec::category::SUBWAY) {
    result.push_back(strategies_->relation_sub_strategy_.get());
    result.push_back(strategies_->sub_strategy_.get());
  }

  // always stub last
  result.push_back(strategies_->stub_strategy_.get());

  return result;
}

std::unique_ptr<relation_strategy> load_relation_strategy(
    strategy_id_t const id, std::vector<station> const& stations,
    std::string const& osm_path, source_spec::category const& category) {
  std::string cache_file{"polylines.path.cache.raw"};

  std::vector<aggregated_polyline> polylines;
  if (fs::is_regular_file(cache_file)) {
    motis::logging::scoped_timer timer("using relations cache");
    polylines = load_relation_polylines(cache_file);
  } else {
    motis::logging::scoped_timer timer("finding relations and building cache");
    auto const relations = parse_relations(osm_path);
    polylines = aggregate_polylines(relations.relations_);
    store_relation_polylines(cache_file, polylines);
  }

  return std::make_unique<relation_strategy>(id, stations, std::move(polylines),
                                             category);
}

std::unique_ptr<rail_strategy> load_rail_strategy(
    strategy_id_t const id, std::vector<station> const& stations,
    std::string const& osm_path, source_spec::category const& category) {
  std::string cache_file{"rail.path.cache.raw"};
  if (category == source_spec::category::SUBWAY) {
    cache_file = "subway.path.cache.raw";
  }

  std::vector<rail_way> rail_ways;
  if (fs::is_regular_file(cache_file)) {
    motis::logging::scoped_timer timer("load rail ways from existing cache");
    rail_ways = load_rail_ways(cache_file);
  } else {
    motis::logging::scoped_timer timer("building rail ways and writing cache");
    if (category == source_spec::category::RAILWAY) {
      rail_ways = build_rail_ways(osm_path);
    } else {
      rail_ways = build_sub_ways(osm_path);
    }
    store_rail_ways(cache_file, rail_ways);
  }

  return std::make_unique<rail_strategy>(id, stations, rail_ways);
}

path_routing make_path_routing(std::vector<station> const& stations,
                               std::string const& osm_path,
                               std::string const& osrm_path) {
  path_routing r;

  strategy_id_t id = 0;

  r.strategies_->osrm_strategy_ =
      std::make_unique<osrm_strategy>(id++, stations, osrm_path);

  r.strategies_->rail_strategy_ = load_rail_strategy(
      id++, stations, osm_path, source_spec::category::RAILWAY);
  r.strategies_->sub_strategy_ = load_rail_strategy(
      id++, stations, osm_path, source_spec::category::SUBWAY);

  r.strategies_->relation_rail_strategy_ = load_relation_strategy(
      id++, stations, osm_path, source_spec::category::RAILWAY);
  r.strategies_->relation_bus_strategy_ = load_relation_strategy(
      id++, stations, osm_path, source_spec::category::BUS);
  r.strategies_->relation_sub_strategy_ = load_relation_strategy(
      id++, stations, osm_path, source_spec::category::SUBWAY);

  r.strategies_->stub_strategy_ =
      std::make_unique<stub_strategy>(id++, stations);

  return r;
}

}  // namespace path
}  // namespace motis