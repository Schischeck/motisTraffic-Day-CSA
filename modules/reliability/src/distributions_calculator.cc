#include "motis/reliability/distributions_calculator.h"

#include <fstream>

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/distributions_container.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/computation/calc_arrival_distribution.h"
#include "motis/reliability/computation/calc_departure_distribution.h"
#include "motis/reliability/computation/data_arrival.h"
#include "motis/reliability/computation/data_departure.h"

namespace motis {
namespace reliability {
namespace distributions_calculator {

namespace common {
void output_element(std::ostream& os, schedule const& schedule,
                    node const& from, node const& to,
                    light_connection const& light_connection,
                    unsigned short const light_connection_idx,
                    bool const is_first_route_node) {
  os << schedule.stations[from._station_node->_id]->name << "(" << from._id
     << ")"
     << " " << format_time(light_connection.d_time) << "--"
     << schedule.categories[light_connection._full_con->con_info->family]->name
     << light_connection._full_con->con_info->train_nr << "("
     << light_connection_idx << ")->" << format_time(light_connection.a_time)
     << " " << schedule.stations[to._station_node->_id]->name << "(" << to._id
     << ")"
     << " first=" << is_first_route_node << " " << &light_connection
     << std::endl;
}

void compute_dep_and_arr_distribution(
    queue_element const& element,
    distributions_container::abstract_distributions_container const&
        train_distributions_container,
    distributions_container::abstract_distributions_container const&
        feeder_distributions_container,
    start_and_travel_distributions const& s_t_distributions,
    schedule const& schedule, probability_distribution& departure_distribution,
    probability_distribution& arrival_distribution) {
  assert(departure_distribution.empty());
  calc_departure_distribution::data_departure d_data(
      *element.from_, *element.light_connection_, element.is_first_route_node_,
      schedule, train_distributions_container, feeder_distributions_container,
      s_t_distributions);
  calc_departure_distribution::compute_departure_distribution(
      d_data, departure_distribution);

  assert(arrival_distribution.empty());
  calc_arrival_distribution::data_arrival a_data(
      *element.to_, *element.light_connection_, departure_distribution,
      schedule, s_t_distributions);
  calc_arrival_distribution::compute_arrival_distribution(a_data,
                                                          arrival_distribution);
}
}  // namespace common

namespace precomputation {
namespace detail {
bool is_pre_computed_route(schedule const& schedule,
                           node const& first_route_node) {
  node const* node = &first_route_node;
  edge const* route_edge = nullptr;
  while ((route_edge = graph_accessor::get_departing_route_edge(*node)) !=
         nullptr) {
    for (unsigned int i = 0; i < route_edge->_m._route_edge._conns.size();
         i++) {
      if (schedule.waiting_time_rules_.other_trains_wait_for(
              schedule.waiting_time_rules_.waiting_time_category(
                  route_edge->_m._route_edge._conns[i]
                      ._full_con->con_info->family))) {
        return true;
      }
    }
    node = route_edge->_to;
  }
  return false;
}

void prepare_distributions_container(
    edge const& route_edge,
    distributions_container::precomputed_distributions_container& container) {
  container.create_route_node_distributions(
      route_edge._from->_id, distributions_container::departure,
      route_edge._m._route_edge._conns.size());
  container.create_route_node_distributions(
      route_edge._to->_id, distributions_container::arrival,
      route_edge._m._route_edge._conns.size());
}

template <bool IsFirstRouteNode, bool CheckClass>
void insert_all_light_connections(
    node const& tail_node, common::queue_type& queue,
    distributions_container::precomputed_distributions_container& container,
    schedule const& schedule) {
  if (!CheckClass || detail::is_pre_computed_route(schedule, tail_node)) {
    auto const route_edge = graph_accessor::get_departing_route_edge(tail_node);
    if (route_edge == nullptr) {
      return;
    }
    prepare_distributions_container(*route_edge, container);
    for (unsigned short light_conn_idx = 0;
         light_conn_idx < route_edge->_m._route_edge._conns.size();
         ++light_conn_idx) {
      auto& light_conn = route_edge->_m._route_edge._conns[light_conn_idx];
      queue.emplace(&tail_node, route_edge->_to, &light_conn, light_conn_idx,
                    IsFirstRouteNode);
    }
  }
}

void process_element(
    common::queue_element const& element, schedule const& schedule,
    start_and_travel_distributions const& s_t_distributions,
    common::queue_type& queue,
    distributions_container::precomputed_distributions_container&
        distributions_container) {
  /* departure distribution */
  auto& departure_distribution =
      distributions_container.get_distribution_non_const(
          element.from_->_id, element.light_connection_idx_,
          distributions_container::departure);
  /* arrival distribution */
  auto& arrival_distribution =
      distributions_container.get_distribution_non_const(
          element.to_->_id, element.light_connection_idx_,
          distributions_container::arrival);

  common::compute_dep_and_arr_distribution(
      element, distributions_container, distributions_container,
      s_t_distributions, schedule, departure_distribution,
      arrival_distribution);
  // insert all light connections out-going from the head-node
  // into the queue. Note that, process element is called
  // for all light-connections of the route-edge.
  // But we have to insert the light connections of the head-node
  // only once into the queue.
  if (element.light_connection_idx_ == 0) {
    detail::insert_all_light_connections<false, false>(
        *element.to_, queue, distributions_container, schedule);
  }
}
}  // namespace detail

void perform_precomputation(
    schedule const& schedule,
    start_and_travel_distributions const& s_t_distributions,
    distributions_container::precomputed_distributions_container&
        distributions_container) {
  common::queue_type queue;

  for (auto const first_route_node : schedule.route_index_to_first_route_node) {
    detail::insert_all_light_connections<true, true>(
        *first_route_node, queue, distributions_container, schedule);
  }

  unsigned int num_processed = 0;
  while (!queue.empty()) {
    detail::process_element(queue.top(), schedule, s_t_distributions, queue,
                            distributions_container);
    queue.pop();
    if (++num_processed % 10000 == 0) {
      std::cout << "." << std::flush;
    }
  }
  std::cout << num_processed << " processed elements" << std::endl;
}
}  // namespace precomputation

namespace ride_distribution {
namespace detail {
void insert_all_elements_into_queue(node const& first_route_node,
                                    unsigned int const light_connection_idx,
                                    node const& last_route_node,
                                    common::queue_type& queue) {
  node const* node = &first_route_node;
  edge const* route_edge = nullptr;
  while ((route_edge = graph_accessor::get_departing_route_edge(*node)) !=
         nullptr) {
    auto& light_conn = route_edge->_m._route_edge._conns[light_connection_idx];
    queue.emplace(node, route_edge->_to, &light_conn, light_connection_idx,
                  node->_id == first_route_node._id);
    if (node == &last_route_node) {
      break;
    }
    node = route_edge->_to;
  }
}
void process_element(
    common::queue_element const& element, schedule const& schedule,
    start_and_travel_distributions const& s_t_distributions,
    distributions_container::precomputed_distributions_container const&
        precomputed_distributions_container,
    distributions_container::ride_distributions_container&
        ride_distributions_container) {
  /* departure distribution */
  auto& departure_distribution =
      ride_distributions_container.create_and_get_distribution_non_const(
          element.from_->_id, element.light_connection_idx_,
          distributions_container::departure);
  /* arrival distribution */
  auto& arrival_distribution =
      ride_distributions_container.create_and_get_distribution_non_const(
          element.to_->_id, element.light_connection_idx_,
          distributions_container::arrival);
  common::compute_dep_and_arr_distribution(
      element, ride_distributions_container,
      precomputed_distributions_container, s_t_distributions, schedule,
      departure_distribution, arrival_distribution);
}
}  // namespace detail

void compute_distributions_for_a_ride(
    unsigned int const light_connection_idx, node const& last_route_node,
    schedule const& schedule,
    start_and_travel_distributions const& s_t_distributions,
    distributions_container::precomputed_distributions_container const&
        precomputed_distributions_container,
    distributions_container::ride_distributions_container&
        ride_distributions_container) {
  node const& very_first_route_node =
      graph_accessor::get_first_route_node(last_route_node);
  common::queue_type queue;
  detail::insert_all_elements_into_queue(
      very_first_route_node, light_connection_idx, last_route_node, queue);
  while (!queue.empty()) {
    detail::process_element(queue.top(), schedule, s_t_distributions,
                            precomputed_distributions_container,
                            ride_distributions_container);
    queue.pop();
  }
}
}  // namespace ride_distribution
}  // namespace distributions_calculator
}  // namespace reliability
}  // namespace motis
