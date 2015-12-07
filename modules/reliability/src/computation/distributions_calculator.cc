#include "motis/reliability/computation/distributions_calculator.h"

#include <fstream>

#include "motis/core/schedule/schedule.h"
#include "motis/core/common/logging.h"

#include "motis/reliability/context.h"
#include "motis/reliability/distributions/distributions_container.h"
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
  os << schedule.stations[from._station_node->_id]->name << "("
     << schedule.stations[from._station_node->_id]->index << "," << from._id
     << ") " << format_time(light_connection.d_time) << "--"
     << schedule.categories[light_connection._full_con->con_info->family]->name
     << light_connection._full_con->con_info->train_nr << "("
     << light_connection_idx << ")->" << format_time(light_connection.a_time)
     << " " << schedule.stations[to._station_node->_id]->name << "(" << to._id
     << ")"
     << " first=" << is_first_route_node << " " << &light_connection
     << std::endl;
}
void output_element(std::ostream& os, schedule const& schedule,
                    queue_element const& element) {
  output_element(os, schedule, *element.from_, *element.to_,
                 *element.light_connection_, element.light_connection_idx_,
                 element.is_first_route_node_);
}

void compute_dep_and_arr_distribution(
    queue_element const& element,
    distributions_container::container const& train_distributions_container,
    context const& context, probability_distribution& departure_distribution,
    probability_distribution& arrival_distribution) {
  assert(departure_distribution.empty());
  calc_departure_distribution::data_departure d_data(
      *element.from_, *element.light_connection_, element.is_first_route_node_,
      train_distributions_container, context);
  calc_departure_distribution::compute_departure_distribution(
      d_data, departure_distribution);

  assert(arrival_distribution.empty());
  calc_arrival_distribution::data_arrival a_data(
      *element.light_connection_, element.from_->get_station()->_id,
      element.to_->get_station()->_id, departure_distribution,
      context.schedule_, context.s_t_distributions_);
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

template <bool IsFirstRouteNode, bool CheckClass>
void insert_all_light_connections(node const& tail_node,
                                  common::queue_type& queue,
                                  schedule const& schedule) {
  if (!CheckClass || detail::is_pre_computed_route(schedule, tail_node)) {
    auto const route_edge = graph_accessor::get_departing_route_edge(tail_node);
    if (route_edge == nullptr) {
      return;
    }
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
    distributions_container::container& distributions_container) {
  /* departure distribution */
  auto& departure_distribution =
      distributions_container.get_distribution_non_const(
          distributions_container::to_container_key(
              *element.light_connection_, element.from_->get_station()->_id,
              time_util::departure, schedule));
  /* arrival distribution */
  auto& arrival_distribution =
      distributions_container.get_distribution_non_const(
          distributions_container::to_container_key(
              *element.light_connection_, element.to_->get_station()->_id,
              time_util::arrival, schedule));

  if (!departure_distribution.empty() || !arrival_distribution.empty()) {
    std::cout << "\nWarning(distributions_calculator): departure or arrival "
                 "distribution already computed: ";
    common::output_element(std::cout, schedule, element);
    return;
  }

  common::compute_dep_and_arr_distribution(
      element, distributions_container,
      context(schedule, distributions_container, s_t_distributions),
      departure_distribution, arrival_distribution);
  // insert all light connections out-going from the head-node
  // into the queue. Note that, process element is called
  // for all light-connections of the route-edge.
  // But we have to insert the light connections of the head-node
  // only once into the queue.
  if (element.light_connection_idx_ == 0) {
    detail::insert_all_light_connections<false, false>(*element.to_, queue,
                                                       schedule);
  }
}
}  // namespace detail

void perform_precomputation(
    schedule const& schedule,
    start_and_travel_distributions const& s_t_distributions,
    distributions_container::container& distributions_container) {
  logging::scoped_timer time("computing distributions");
  common::queue_type queue;

  for (auto const first_route_node : schedule.route_index_to_first_route_node) {
    detail::insert_all_light_connections<true, true>(*first_route_node, queue,
                                                     schedule);
  }

  while (!queue.empty()) {
    detail::process_element(queue.top(), schedule, s_t_distributions, queue,
                            distributions_container);
    queue.pop();
  }
}

}  // namespace precomputation
}  // namespace distributions_calculator
}  // namespace reliability
}  // namespace motis
