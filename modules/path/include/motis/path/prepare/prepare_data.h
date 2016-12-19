#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "geo/polygon.h"

#include "motis/path/prepare/routing/relation_routing.h"
#include "motis/path/prepare/routing/routing_strategy.h"
#include "motis/path/prepare/routing/stub_routing.h"
#include "motis/path/prepare/source_spec.h"

namespace motis {
namespace path {

struct kv_database;
struct station_seq;

struct strategies {
  strategies(std::unique_ptr<stub_routing> stub,
             std::unique_ptr<relation_routing> relation)
      : stub_strategy_(std::move(stub)),
        relation_strategy_(std::move(relation)) {
    strategies_.push_back(stub_strategy_.get());
    strategies_.push_back(relation_strategy_.get());
  }

  std::unique_ptr<stub_routing> stub_strategy_;
  std::unique_ptr<relation_routing> relation_strategy_;
  std::vector<routing_strategy*> strategies_;
  std::map<source_spec::category, routing_strategy*> class_to_strategy_;
};

void strategy_prepare(std::vector<station_seq> const& sequences,
                      strategies& routing_strategies, kv_database& db);

std::vector<routing_strategy*> get_strategies(
    strategies const& routing_strategies,
    source_spec::category const& category);

}  // namespace path
}  // namespace motis