#pragma once

#include <vector>

#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/nodes.h"

#include "motis/reliability/distributions/distributions_container.h"
#include "motis/reliability/realtime/time_util.h"

namespace motis {
namespace reliability {
namespace realtime {
namespace dependencies_finder {
struct dependency {
  node const* route_node_;
  light_connection const* light_connection_;
  time_util::event_type type_;
};

std::vector<dependency> find_dependencies(
    distributions_container::container::key const&) {
  std::vector<dependency> dependencies;

  return dependencies;
}
}
}
}
}
