#include "motis/reliability/rating/public_transport.h"

#include "motis/core/schedule/schedule.h"

#include "motis/protocol/RoutingResponse_generated.h"

#include "motis/reliability/distributions_calculator.h"
#include "motis/reliability/distributions_container.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/probability_distribution.h"

namespace motis {
namespace reliability {
using distributions_calculator::common::queue_element;

namespace rating {
namespace public_transport {

void distribution_for_first_train(
    probability_distribution& departure_distribution,
    probability_distribution& arrival_distribution,
    queue_element const& element,
    distributions_container::precomputed_distributions_container const&
        precomputed_distributions,
    distributions_container::ride_distributions_container& ride_distributions,
    bool const is_first_event) {
  if (precomputed_distributions.contains_distributions(
          element.from_->_id, distributions_container::departure)) {
    departure_distribution = precomputed_distributions.get_distribution(
        element.from_->_id, element.light_connection_idx_,
        distributions_container::departure);
    arrival_distribution = precomputed_distributions.get_distribution(
        element.to_->_id, element.light_connection_idx_,
        distributions_container::arrival);
  } else {
    if (is_first_event) {
      // distributions_calculator::ride_distribution::compute_distributions_for_a_ride()
    }
  }
  assert(false);
}

std::vector<probability_distribution> const rate(
    std::vector<queue_element> const& elements, schedule const& schedule,
    distributions_container::precomputed_distributions_container const&
        precomputed_distributions) {
  std::vector<probability_distribution> distributions;
  distributions_container::ride_distributions_container ride_distributions;

  bool is_first_train = true; /* true until the first interchange is reached */
  for (unsigned int index = 0; index < distributions.size(); ++index) {
    auto const& element = elements[index];
    probability_distribution departure_distribution, arrival_distribution;

    bool const is_interchange =
        index > 0 && elements[index - 1].to_->_id != element.from_->_id;
    if (is_interchange && is_first_train) {
      is_first_train = false;
    }

    if (is_first_train) {
      distribution_for_first_train(departure_distribution, arrival_distribution,
                                   element, precomputed_distributions,
                                   ride_distributions, index == 0);
    }
    distributions.push_back(departure_distribution);
    distributions.push_back(arrival_distribution);
  }

  return distributions;
}

}  // namespace public_transport
}  // namespace rating
}  // namespace reliability
}  // namespace motis
