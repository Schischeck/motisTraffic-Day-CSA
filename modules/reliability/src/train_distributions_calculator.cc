#include "motis/reliability/train_distributions_calculator.h"

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/calc_arrival_distribution.h"
#include "motis/reliability/calc_departure_distribution.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/pd_calc_data_departure.h"
#include "motis/reliability/pd_calc_data_arrival.h"
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

void train_distributions_calculator::calculate_initial_distributions() {
  for (auto first_route_node : schedule_.route_index_to_first_route_node) {
    insert_all_light_connections(*first_route_node, true);
  }

  while (!queue_.empty()) {
    process_element(queue_.top());
    queue_.pop();
  }
}

void train_distributions_calculator::process_element(queue_element element) {
  /* departure distribution */
  auto& departure_distribution =
      distributions_container_.get_train_distributions(element.from_->_id)
          .departure_distributions_[element.light_connection_idx_];
  pd_calc_data_departure d_data(
      *element.from_, *element.light_connection_, element.is_first_route_node_,
      schedule_, distributions_container_, tt_distributions_manager_);
  calc_departure_distribution::compute_departure_distribution(
      d_data, departure_distribution);

  /* arrival distribution */
  auto& arrival_distribution =
      distributions_container_.get_train_distributions(element.to_->_id)
          .arrival_distributions_[element.light_connection_idx_];
  pd_calc_data_arrival a_data(*element.to_, *element.light_connection_,
                              schedule_, distributions_container_,
                              tt_distributions_manager_);
  calc_arrival_distribution::compute_arrival_distribution(a_data,
                                                          arrival_distribution);

  insert_all_light_connections(*element.to_, false);
}

bool compute_distributions(schedule const& schedule, edge const& route_edge) {
  return schedule.waiting_time_rules_.other_trains_wait_for(
      schedule.waiting_time_rules_.waiting_time_category(
          route_edge._m._route_edge._conns[0]._full_con->con_info->family));
}

void train_distributions_calculator::insert_all_light_connections(
    node& tail_node, bool const is_first_route_node) {
  auto& route_edge = *graph_accessor::get_departing_route_edge(tail_node);
  if (compute_distributions(schedule_, route_edge)) {

    assert(distributions_container_.get_train_distributions(tail_node._id)
               .departure_distributions_.size() == 0);
    distributions_container_.get_train_distributions(tail_node._id)
        .departure_distributions_.reserve(
            route_edge._m._route_edge._conns.size());

    assert(distributions_container_.get_train_distributions(route_edge._to->_id)
               .arrival_distributions_.size() == 0);
    distributions_container_.get_train_distributions(route_edge._to->_id)
        .arrival_distributions_.reserve(
            route_edge._m._route_edge._conns.size());

    for (unsigned short light_conn_idx = 0;
         light_conn_idx < route_edge._m._route_edge._conns.size();
         ++light_conn_idx) {
      auto& light_conn = route_edge._m._route_edge._conns[light_conn_idx];
      queue_.emplace(&tail_node, route_edge._to, &light_conn, light_conn_idx,
                     is_first_route_node);
    }
  }
}

}  // namespace reliability
}  // namespace motis
