#include "motis/reliability/train_distributions_calculator.h"

#include "motis/core/schedule/Schedule.h"

using namespace td;

namespace motis {
namespace reliability {

train_distributions_calculator::train_distributions_calculator(
    Schedule* schedule)
    : schedule_(schedule) {}

bool train_distributions_calculator::calculate_initial_distributions() {
  return insert_first_route_elements_into_queue() && process_queue();
}

Edge* get_route_edge(Node* route_node) {
  for (auto& edge : route_node->_edges) {
    if (!edge.empty()) {
      return &edge;
    }
  }
  return nullptr;
}

bool train_distributions_calculator::insert_first_route_elements_into_queue() {
  for (auto& first_route_node : schedule_->routeIndexToFirstRouteNode) {
    auto route_edge = get_route_edge(first_route_node);
    for (auto& light_conn : route_edge->_m._routeEdge._conns) {
      queue_.emplace(&light_conn, first_route_node, route_edge->_to);
    }
  }
  return true;
}

bool train_distributions_calculator::process_queue() { return true; }

}  // namespace reliability
}  // namespace motis
