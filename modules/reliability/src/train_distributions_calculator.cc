#include "motis/reliability/train_distributions_calculator.h"

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/pd_calc_data_departure.h"
#include "motis/reliability/train_distributions.h"
#include "motis/reliability/tt_distributions_manager.h"

namespace motis {
namespace reliability {

train_distributions_calculator::train_distributions_calculator(
    schedule& schedule, train_distributions_container& distributions_container,
    tt_distributions_manager& tt_dist_manager)
    : schedule_(schedule),
      distributions_container_(distributions_container),
      tt_distributions_manager_(tt_dist_manager) {}

bool train_distributions_calculator::calculate_initial_distributions() {
  insert_first_route_elements_into_queue();

  while (!queue_.empty()) {
    if (!process_element(queue_.top())) return false;
    queue_.pop();
  }

  return true;
}

bool compute_distributions(schedule const& schedule, edge const& route_edge) {
  return schedule.waiting_time_rules_.other_trains_wait_for(
      schedule.waiting_time_rules_.waiting_time_category(
          route_edge._m._route_edge._conns[0]._full_con->con_info->family));
}

bool train_distributions_calculator::insert_first_route_elements_into_queue() {
  for (auto first_route_node : schedule_.route_index_to_first_route_node) {
    auto route_edge =
        graph_accessor::get_departing_route_edge(*first_route_node);
    if (compute_distributions(schedule_, *route_edge)) {
      for (unsigned short light_conn_idx = 0;
           light_conn_idx < route_edge->_m._route_edge._conns.size();
           ++light_conn_idx) {
        auto& light_conn = route_edge->_m._route_edge._conns[light_conn_idx];
        queue_.emplace(first_route_node, route_edge->_to, &light_conn,
                       light_conn_idx, true);
      }
    }
  }
  return true;
}

bool train_distributions_calculator::process_element(queue_element element) {
#if 0
  pd_calc_data_departure d_data(
      *element.from_, *element.light_connection_, element.is_first_route_node_,
      schedule_, distributions_container_, tt_distributions_manager_);
  probability_distribution departure_distribution;
  // pd_calculator.calc_prob_dist_departure(departure_distribution, d_data);

  pd_calc_data_arrival a_data(element.to_, element.light_connection_, schedule_,
                              distributions_container_,
                              tt_distributions_manager_);
  probability_distribution arrival_distribution;
  // pd_calculator.calc_prob_dist_arrival(arrival_distribution, d_data);

  // TODO add successors
#endif
  return true;
}

}  // namespace reliability
}  // namespace motis
