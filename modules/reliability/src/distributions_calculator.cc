#include <fstream>

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/calc_arrival_distribution.h"
#include "motis/reliability/calc_departure_distribution.h"
#include "motis/reliability/distributions_calculator.h"
#include "motis/reliability/distributions_container.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/pd_calc_data_departure.h"
#include "motis/reliability/pd_calc_data_arrival.h"

namespace motis {
namespace reliability {

namespace distributions_calculator {

void output_element(std::ostream& os, schedule const& schedule,
                    node const& from, node const& to,
                    light_connection const& light_connection,
                    unsigned short const light_connection_idx,
                    bool const is_first_route_node) {
  os << schedule.stations[from._station_node->_id]->name << "(" << from._id
     << ")"
     << " " << format_time(light_connection.d_time) << "--"
     << schedule.category_names[light_connection._full_con->con_info->family]
     << light_connection._full_con->con_info->train_nr << "("
     << light_connection_idx << ")->" << format_time(light_connection.a_time)
     << " " << schedule.stations[to._station_node->_id]->name << "(" << to._id
     << ")"
     << " first=" << is_first_route_node << " " << &light_connection
     << std::endl;
}

void compute_dep_and_arr_distribution(
    queue_element const& element,
    precomputed_distributions_container& container,
    start_and_travel_distributions const& s_t_distributions,
    schedule const& schedule) {
  /* departure distribution */
  auto& departure_distribution =
      container.get_route_node_distributions(
                    element.from_->_id,
                    precomputed_distributions_container::departure)
          .get_distribution_non_const(element.light_connection_idx_);
  assert(departure_distribution.empty());
  pd_calc_data_departure d_data(*element.from_, *element.light_connection_,
                                element.is_first_route_node_, schedule,
                                container, s_t_distributions);
  calc_departure_distribution::compute_departure_distribution(
      d_data, departure_distribution);

  /* arrival distribution */
  auto& arrival_distribution =
      container.get_route_node_distributions(
                    element.to_->_id,
                    precomputed_distributions_container::arrival)
          .get_distribution_non_const(element.light_connection_idx_);
  assert(arrival_distribution.empty());
  pd_calc_data_arrival a_data(*element.to_, *element.light_connection_,
                              departure_distribution, schedule,
                              s_t_distributions);
  calc_arrival_distribution::compute_arrival_distribution(a_data,
                                                          arrival_distribution);
}

template <bool FirstRouteNode, bool CheckClass>
void insert_all_light_connections(
    node const& tail_node, queue_type& queue,
    precomputed_distributions_container& container, schedule const& schedule) {
  if (!CheckClass ||
      distributions_calculator::is_pre_computed_route(schedule, tail_node)) {
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
                    FirstRouteNode);
    }
  }
}

void prepare_distributions_container(
    edge const& route_edge, precomputed_distributions_container& container) {
  container.create_route_node_distributions(
      route_edge._from->_id, precomputed_distributions_container::departure,
      route_edge._m._route_edge._conns.size());
  container.create_route_node_distributions(
      route_edge._to->_id, precomputed_distributions_container::arrival,
      route_edge._m._route_edge._conns.size());
}

bool is_pre_computed_route(schedule const& schedule,
                           node const& first_route_node) {
  node const* node = &first_route_node;
  edge const* route_edge = nullptr;
  while ((route_edge = graph_accessor::get_departing_route_edge(*node)) !=
         nullptr) {
    if (schedule.waiting_time_rules_.other_trains_wait_for(
            schedule.waiting_time_rules_.waiting_time_category(
                route_edge->_m._route_edge._conns[0]
                    ._full_con->con_info->family))) {
      return true;
    }
    node = route_edge->_to;
  }
  return false;
}

}  // namespace distributions_calculator

void precomputed_distributions_calculator::perform_precomputation(
    schedule const& schedule,
    precomputed_distributions_container& distributions_container,
    start_and_travel_distributions const& s_t_distributions) {
  distributions_calculator::queue_type queue;

  for (auto const first_route_node : schedule.route_index_to_first_route_node) {
    distributions_calculator::insert_all_light_connections<true, true>(
        *first_route_node, queue, distributions_container, schedule);
  }

  unsigned int num_processed = 0;

  while (!queue.empty()) {
    auto const& element = queue.top();
    distributions_calculator::compute_dep_and_arr_distribution(
        element, distributions_container, s_t_distributions, schedule);
    // insert all light connections out-going from the head-node
    // into the queue. Note that, process element is called
    // for all light-connections of the route-edge.
    // But we have to insert the light connections of the head-node
    // only once into the queue.
    if (element.light_connection_idx_ == 0) {
      distributions_calculator::insert_all_light_connections<false, false>(
          *element.to_, queue, distributions_container, schedule);
    }
    queue.pop();
    if (++num_processed % 10000 == 0) {
      std::cout << "." << std::flush;
    }
  }

  std::cout << num_processed - 1 << " processed elements" << std::endl;
}

}  // namespace reliability
}  // namespace motis
