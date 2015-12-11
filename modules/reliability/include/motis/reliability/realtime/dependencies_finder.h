#pragma once

#include <vector>

#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/nodes.h"

#include "motis/reliability/graph_accessor.h"
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

namespace detail {
void add_dependencies(edge const& route_edge,
                      distributions_container::container::key const& key,
                      std::vector<dependency>& dependencies) {}
}

std::vector<dependency> find_dependencies(
    distributions_container::container::key const& key, schedule const& sched) {
  std::vector<dependency> dependencies;

  auto const& station_node = sched.station_nodes.at(key.station_index_);

  std::for_each(station_node->_edges.begin(), station_node->_edges.end(),
                [&](edge const& edge) {
                  if (!edge._to->is_route_node()) {
                    return;
                  }
                  detail::add_dependencies(
                      *graph_accessor::get_departing_route_edge(*edge._to), key,
                      dependencies);
                });

  return dependencies;
}
}
}
}
}
