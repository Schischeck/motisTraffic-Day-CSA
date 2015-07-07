#include "motis/reliability/train_distributions_calculator.h"

#include "motis/core/schedule/Schedule.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/pd_calc_data.h"
#include "motis/reliability/train_distributions.h"

using namespace td;

namespace motis {
namespace reliability {

train_distributions_calculator::train_distributions_calculator(
    Schedule* schedule, train_distributions_container& distributions_container)
    : schedule_(schedule), distributions_container_(distributions_container) {}

bool train_distributions_calculator::calculate_initial_distributions() {
  insert_first_route_elements_into_queue();

  while (!queue_.empty()) {
    if (!process_element(queue_.top())) return false;
    queue_.pop();
  }

  return true;
}

bool train_distributions_calculator::insert_first_route_elements_into_queue() {
  for (auto first_route_node : schedule_->routeIndexToFirstRouteNode) {
    auto route_edge =
        graph_accessor::get_departing_route_edge(*first_route_node);
    for (unsigned short light_conn_idx = 0;
         light_conn_idx < route_edge->_m._routeEdge._conns.size();
         ++light_conn_idx) {
      auto& light_conn = route_edge->_m._routeEdge._conns[light_conn_idx];
      queue_.emplace(first_route_node, route_edge->_to, &light_conn,
                     light_conn_idx, true);
    }
  }
  return true;
}

bool train_distributions_calculator::process_element(queue_element element) {
#if 0
  pd_calc_data_departure d_data(*element.from_, *element.light_connection_);
  pd_calculator.calc_prob_dist_departure(
      element.from_->departure_distributions[element.light_connection_idx_], d_data);

  pd_calc_data_arr a_data(element.to_, element.light_connection_);
  pd_calculator.calc_prob_dist_arrival(
        element.to_->arrival_distributions[element.light_connection_idx_], d_data);

  // TODO add successors

#endif
  return true;
}

}  // namespace reliability
}  // namespace motis
