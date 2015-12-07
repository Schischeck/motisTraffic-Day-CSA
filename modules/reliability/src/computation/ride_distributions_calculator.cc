#include "motis/reliability/computation/ride_distributions_calculator.h"

#include <cassert>

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/context.h"
#include "motis/reliability/computation/distributions_calculator.h"
#include "motis/reliability/distributions/distributions_container.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/rating/connection_rating.h"
#include "motis/reliability/rating/public_transport.h"

namespace motis {
namespace reliability {
namespace distributions_calculator {
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
    common::queue_element const& element, context const& context,
    distributions_container::container& ride_distributions_container) {
  /* departure distribution */
  auto& departure_distribution =
      ride_distributions_container.get_distribution_non_const(
          distributions_container::to_container_key(
              *element.light_connection_, element.from_->get_station()->_id,
              time_util::departure, context.schedule_));
  /* arrival distribution */
  auto& arrival_distribution =
      ride_distributions_container.get_distribution_non_const(
          distributions_container::to_container_key(
              *element.light_connection_, element.to_->get_station()->_id,
              time_util::arrival, context.schedule_));
  common::compute_dep_and_arr_distribution(
      element, ride_distributions_container, context, departure_distribution,
      arrival_distribution);
}
void compute_distributions_for_a_ride(
    unsigned int const light_connection_idx, node const& last_route_node,
    context const& context,
    distributions_container::container& ride_distributions_container) {
  node const& very_first_route_node =
      graph_accessor::get_first_route_node(last_route_node);
  common::queue_type queue;
  detail::insert_all_elements_into_queue(
      very_first_route_node, light_connection_idx, last_route_node, queue);
  while (!queue.empty()) {
    detail::process_element(queue.top(), context, ride_distributions_container);
    queue.pop();
  }
}

}  // namespace detail

std::vector<bool> compute_missing_train_distributions(
    distributions_container::container& ride_distributions,
    std::vector<std::vector<rating::connection_element>> const& trains,
    context const& context) {
  std::vector<bool> precomputed_flags;

  /* first train */
  auto const& first_element = trains.front().front();
  precomputed_flags.push_back(
      context.precomputed_distributions_.contains_distribution(
          distributions_container::to_container_key(
              *first_element.light_connection_,
              first_element.from_->get_station()->_id, time_util::departure,
              context.schedule_)));

  /* For the first train, we have to compute the distributions of all events
   * of that train in the connection. */
  if (!precomputed_flags.front()) {
    // last element of the first train
    auto const& last_element = trains.front().back();
    detail::compute_distributions_for_a_ride(last_element.light_connection_idx_,
                                             *last_element.to_, context,
                                             ride_distributions);
  }

  /* For the next trains, we only need the arrival distribution of the arrival
   * event before the first departure event of that train in the connection */
  for (auto train_elements :
       std::vector<std::vector<rating::connection_element>>(trains.begin() + 1,
                                                            trains.end())) {
    auto const& element = train_elements.front();
    precomputed_flags.push_back(
        context.precomputed_distributions_.contains_distribution(
            distributions_container::to_container_key(
                *element.light_connection_, element.from_->get_station()->_id,
                time_util::departure, context.schedule_)));
    if (!precomputed_flags.back()) {
      detail::compute_distributions_for_a_ride(element.light_connection_idx_,
                                               *element.from_, context,
                                               ride_distributions);
    }
  }
  assert(precomputed_flags.size() == trains.size());
  return precomputed_flags;
}

}  // namespace ride_distribution
}  // namespace distributions_calculator
}  // namespace reliability
}  // namespace motis
