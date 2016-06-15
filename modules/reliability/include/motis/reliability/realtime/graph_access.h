#pragma once

#include "motis/reliability/distributions/distributions_container.h"

namespace motis {
struct schedule;
class node;
struct light_connection;
namespace reliability {
namespace realtime {
namespace graph_access {
std::pair<node const*, light_connection const*> get_node_and_light_connection(
    distributions_container::container::key const&, schedule const&);
}  // namespace graph_access
}  // namespace realtime
}  // namespace reliability
}  // namespace motis
