#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "geo/polygon.h"

#include "motis/routes/prepare/routing/routing_strategy.h"
#include "motis/routes/prepare/source_spec.h"

namespace motis {
namespace routes {

struct kv_database;
struct station_seq;

struct strategies {
  std::vector<std::unique_ptr<routing_strategy>> strategies_;
  std::map<source_spec::category, routing_strategy*> class_to_strategy_;
};

struct prepare_data {
  std::vector<station_seq> sequences_;
  std::map<std::string, std::vector<geo::latlng>> stop_positions_;
};

void prepare(prepare_data& data, strategies& routing_strategies,
             kv_database& db, std::string const& osm);

routing_strategy* get_strategy(strategies const& routing_strategies,
                               source_spec::category const& category);

}  // namespace routes
}  // namespace motis