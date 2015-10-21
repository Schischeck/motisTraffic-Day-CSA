#include "motis/reliability/rating/public_transport.h"

#include "motis/core/schedule/schedule.h"

#include "motis/protocol/RoutingResponse_generated.h"

#include "motis/reliability/computation/calc_arrival_distribution.h"
#include "motis/reliability/computation/calc_departure_distribution.h"
#include "motis/reliability/computation/calc_departure_distribution_interchange.h"
#include "motis/reliability/computation/data_arrival.h"
#include "motis/reliability/computation/data_departure_interchange.h"
#include "motis/reliability/distributions_calculator.h"
#include "motis/reliability/distributions_container.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/probability_distribution.h"
#include "motis/reliability/rating/connection_rating.h"

namespace motis {
namespace reliability {
namespace rating {
namespace public_transport {

/* fills ride_distributions with distributions for events in the connection
 * that have not been pre-computed
 * @return for each train, return if the distributions were pre-computed */
std::vector<bool> compute_missing_train_distributions(
    distributions_container::ride_distributions_container& ride_distributions,
    std::vector<std::vector<connection_element>> const& elements,
    distributions_container::precomputed_distributions_container const&
        precomputed_distributions,
    schedule const& schedule,
    start_and_travel_distributions const& s_t_distributions) {
  std::vector<bool> precomputed_flags;

  // first train
  precomputed_flags.push_back(precomputed_distributions.contains_distributions(
      elements.front().front().from_->_id, distributions_container::departure));

  /* For the first train, we have to compute the distributions of all events
   * of that train in the connection. */
  if (!precomputed_flags.front()) {
    // last element of the first train
    auto const& last_element = elements.front().back();
    distributions_calculator::ride_distribution::
        compute_distributions_for_a_ride(
            last_element.light_connection_idx_, *last_element.to_, schedule,
            s_t_distributions, precomputed_distributions, ride_distributions);
  }

  /* For the next trains, we only need the arrival distribution of the arrival
   * event before the first departure event of that train in the connection */
  for (auto train_elements : std::vector<std::vector<connection_element>>(
           elements.begin() + 1, elements.end())) {
    auto const& element = train_elements.front();
    precomputed_flags.push_back(
        precomputed_distributions.contains_distributions(
            element.from_->_id, distributions_container::departure));
    if (!precomputed_flags.back()) {
      distributions_calculator::ride_distribution::
          compute_distributions_for_a_ride(
              element.light_connection_idx_, *element.from_, schedule,
              s_t_distributions, precomputed_distributions, ride_distributions);
    }
  }
  assert(precomputed_flags.size() == elements.size());
  return precomputed_flags;
}

void distributions_for_first_train(
    std::vector<rating_element>& ratings,
    std::vector<connection_element> const& elements,
    distributions_container::abstract_distributions_container const&
        distributions_container) {
  for (auto const& element : elements) {
    ratings.emplace_back(element.departure_stop_idx_);
    ratings.back().departure_distribution_ =
        distributions_container.get_distribution(
            element.from_->_id, element.light_connection_idx_,
            distributions_container::departure);
    ratings.back().arrival_distribution_ =
        distributions_container.get_distribution(
            element.to_->_id, element.light_connection_idx_,
            distributions_container::arrival);
  }
}

std::unique_ptr<calc_departure_distribution::data_departure_interchange>
create_data_for_interchange(
    connection_element const& element,
    connection_element const& preceding_element,
    probability_distribution const& arrival_distribution,
    distributions_container::abstract_distributions_container const&
        train_distributions,
    distributions_container::precomputed_distributions_container const&
        feeder_distributions,
    schedule const& schedule,
    start_and_travel_distributions const& s_t_distributions) {
  // interchange with walk
  if (preceding_element.to_->_station_node->_id !=
      element.from_->_station_node->_id) {
    return std::unique_ptr<
        calc_departure_distribution::data_departure_interchange>(
        new calc_departure_distribution::data_departure_interchange_walk(
            element.is_first_route_node_, *element.from_,
            *preceding_element.to_->_station_node, *element.light_connection_,
            *preceding_element.light_connection_, arrival_distribution,
            schedule, train_distributions, feeder_distributions,
            s_t_distributions));
  }
  // interchange without walk
  return std::unique_ptr<
      calc_departure_distribution::data_departure_interchange>(
      new calc_departure_distribution::data_departure_interchange(
          element.is_first_route_node_, *element.from_,
          *element.light_connection_, *preceding_element.light_connection_,
          arrival_distribution, schedule, train_distributions,
          feeder_distributions, s_t_distributions));
}

void distributions_for_train_after_interchange(
    std::vector<rating_element>& ratings,
    std::vector<connection_element> const& elements,
    connection_element preceding_element,
    distributions_container::abstract_distributions_container const&
        train_distributions,
    distributions_container::precomputed_distributions_container const&
        feeder_distributions,
    schedule const& schedule,
    start_and_travel_distributions const& s_t_distributions) {
  for (auto const& element : elements) {
    ratings.emplace_back(element.departure_stop_idx_);
    auto const& preceding_arrival_distribution =
        ratings[ratings.size() - 2].arrival_distribution_;
    auto& departure_distribution = ratings.back().departure_distribution_;
    auto& arrival_distribution = ratings.back().arrival_distribution_;

    if (&element == &elements.front()) { /* departure with interchange */
      auto dep_data = create_data_for_interchange(
          elements.front(), preceding_element, preceding_arrival_distribution,
          train_distributions, feeder_distributions, schedule,
          s_t_distributions);
      calc_departure_distribution::interchange::compute_departure_distribution(
          *dep_data, departure_distribution);
    } else { /* departure without interchange */
      auto dep_data =
          std::unique_ptr<calc_departure_distribution::data_departure>(
              new calc_departure_distribution::data_departure(
                  *element.from_, *element.light_connection_,
                  element.is_first_route_node_, schedule,
                  distributions_container::single_distribution_container(
                      preceding_arrival_distribution),
                  feeder_distributions, s_t_distributions));
      calc_departure_distribution::compute_departure_distribution(
          *dep_data, departure_distribution);
    }

    calc_arrival_distribution::data_arrival arr_data(
        *element.to_, *element.light_connection_, departure_distribution,
        schedule, s_t_distributions);
    calc_arrival_distribution::compute_arrival_distribution(
        arr_data, arrival_distribution);
  }
}

void rate(std::vector<rating_element>& ratings,
          std::vector<std::vector<connection_element>> const& elements,
          schedule const& schedule,
          distributions_container::precomputed_distributions_container const&
              precomputed_distributions,
          start_and_travel_distributions const& s_t_distributions) {
  assert(elements.size() > 0);

  distributions_container::ride_distributions_container ride_distributions;
  auto const& precomputed_flags = compute_missing_train_distributions(
      ride_distributions, elements, precomputed_distributions, schedule,
      s_t_distributions);

  for (unsigned int train_idx = 0; train_idx < elements.size(); ++train_idx) {
    auto const& train_distributions =
        precomputed_flags.at(train_idx)
            ? dynamic_cast<distributions_container::
                               abstract_distributions_container const&>(
                  precomputed_distributions)
            : dynamic_cast<distributions_container::
                               abstract_distributions_container const&>(
                  ride_distributions);

    if (train_idx == 0) {
      distributions_for_first_train(ratings, elements.front(),
                                    train_distributions);
    } else {
      distributions_for_train_after_interchange(
          ratings, elements[train_idx], elements[train_idx - 1].back(),
          train_distributions, precomputed_distributions, schedule,
          s_t_distributions);
    }
  }
}

}  // namespace public_transport
}  // namespace rating
}  // namespace reliability
}  // namespace motis
