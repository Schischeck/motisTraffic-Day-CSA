#pragma once

#include <memory>

#include "motis/path/prepare/routing/routing_strategy.h"

namespace motis {
namespace path {

struct path_routing {
  path_routing();
  ~path_routing();

  path_routing(path_routing&&) = default;
  path_routing& operator=(path_routing&&) = default;

  std::vector<routing_strategy const*> strategies_for() const;

  struct impl;
  std::unique_ptr<impl> impl_;
};

path_routing make_path_routing();

} // namespace path
} // namespace motis