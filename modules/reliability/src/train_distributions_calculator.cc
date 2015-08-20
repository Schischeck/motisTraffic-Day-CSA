#include "motis/reliability/train_distributions_calculator.h"

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/calc_arrival_distribution.h"
#include "motis/reliability/calc_departure_distribution.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/pd_calc_data_departure.h"
#include "motis/reliability/pd_calc_data_arrival.h"
#include "motis/reliability/train_distributions.h"

namespace motis {
namespace reliability {

train_distributions_calculator::train_distributions_calculator(
    schedule const& schedule,
    train_distributions_container& distributions_container,
    start_and_travel_distributions const& s_t_dist_manager)
    : schedule_(schedule),
      distributions_container_(distributions_container),
      s_t_distributions_(s_t_dist_manager) {}

void train_distributions_calculator::calculate_initial_distributions() {
  for (auto const first_route_node :
       schedule_.route_index_to_first_route_node) {
    insert_all_light_connections(*first_route_node, true);
  }

  while (!queue_.empty()) {
    process_element(queue_.top());
    queue_.pop();
  }
}

void train_distributions_calculator::output_element(
    std::ostream& os, schedule const& schedule, node const& from,
    node const& to, light_connection const& light_connection,
    unsigned short const light_connection_idx, bool const is_first_route_node) {
  os << schedule.stations[from._station_node->_id]->name << "(" << from._id
     << ")"
     << " " << format_time(light_connection.d_time) << "--"
     << schedule.category_names[light_connection._full_con->con_info->family]
     << light_connection._full_con->con_info->train_nr << "->"
     << format_time(light_connection.a_time) << " "
     << schedule.stations[to._station_node->_id]->name << "(" << to._id << ")"
     << " first=" << is_first_route_node << std::endl;
}

void train_distributions_calculator::insert_into_queue(
    node const* from, node const* to, light_connection const* light_connection,
    unsigned short const light_connection_idx, bool const is_first_route_node) {
  queue_.emplace(from, to, light_connection, light_connection_idx,
                 is_first_route_node);
}

void train_distributions_calculator::process_element(
    queue_element const& element) {
  /* departure distribution */
  auto& departure_distribution =
      distributions_container_.get_train_distributions(
                                   element.from_->_id,
                                   train_distributions_container::departure)
          .get_distribution_non_const(element.light_connection_idx_);
  assert(departure_distribution.empty());
  pd_calc_data_departure d_data(*element.from_, *element.light_connection_,
                                element.is_first_route_node_, schedule_,
                                distributions_container_, s_t_distributions_);
  calc_departure_distribution::compute_departure_distribution(
      d_data, departure_distribution);

  /* arrival distribution */
  auto& arrival_distribution =
      distributions_container_.get_train_distributions(
                                   element.to_->_id,
                                   train_distributions_container::arrival)
          .get_distribution_non_const(element.light_connection_idx_);
  assert(arrival_distribution.empty());
  pd_calc_data_arrival a_data(*element.to_, *element.light_connection_,
                              schedule_, distributions_container_,
                              s_t_distributions_);
  calc_arrival_distribution::compute_arrival_distribution(a_data,
                                                          arrival_distribution);

  // insert all light connections out-going from the head-node
  // into the queue. Note that, process element is called
  // for all light-connections of the route-edge.
  // But we have to insert the light connections of the head-node
  // only once into the queue.
  if (element.light_connection_idx_ == 0) {
    insert_all_light_connections(*element.to_, false);
  }
}

bool train_distributions_calculator::compute_distributions(
    schedule const& schedule, edge const& route_edge) {
  return schedule.waiting_time_rules_.other_trains_wait_for(
      schedule.waiting_time_rules_.waiting_time_category(
          route_edge._m._route_edge._conns[0]._full_con->con_info->family));
}

void train_distributions_calculator::insert_all_light_connections(
    node const& tail_node, bool const is_first_route_node) {
  auto const route_edge = graph_accessor::get_departing_route_edge(tail_node);
  if (route_edge == nullptr) {
    return;
  }

  if (compute_distributions(schedule_, *route_edge)) {
    manage_train_distributions(*route_edge);
    for (unsigned short light_conn_idx = 0;
         light_conn_idx < route_edge->_m._route_edge._conns.size();
         ++light_conn_idx) {
      auto& light_conn = route_edge->_m._route_edge._conns[light_conn_idx];
      insert_into_queue(&tail_node, route_edge->_to, &light_conn,
                        light_conn_idx, is_first_route_node);
    }
  }
}

void train_distributions_calculator::manage_train_distributions(
    edge const& route_edge) {
  distributions_container_.create_train_distributions(
      route_edge._from->_id, train_distributions_container::departure,
      route_edge._m._route_edge._conns.size());
  distributions_container_.create_train_distributions(
      route_edge._to->_id, train_distributions_container::arrival,
      route_edge._m._route_edge._conns.size());
}

}  // namespace reliability
}  // namespace motis
