#pragma once

#include <memory>

#include "motis/path/prepare/routing/routing_strategy.h"

namespace motis {
namespace path {

struct routing {
  routing();
  ~routing();

  std::vector<routing_strategy const*> strategies_for() const;

  struct impl;
  std::unique_ptr<impl> impl_;
};

routing make_routing();

} // namespace path
} // namespace motis