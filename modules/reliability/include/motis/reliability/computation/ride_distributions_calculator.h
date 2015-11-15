#pragma once

#include <vector>

namespace motis {
struct schedule;
class node;

namespace reliability {
struct context;
struct start_and_travel_distributions;
namespace distributions_container {
struct precomputed_distributions_container;
struct ride_distributions_container;
}
namespace rating {
struct connection_element;
};

namespace distributions_calculator {
namespace ride_distribution {

/* fills ride_distributions with distributions for events in the connection
 * that have not been pre-computed
 * @param trains: for each train in the connection, a vector of
 * connection-elements
 * @return for each train, return if the distributions were pre-computed */
std::vector<bool> compute_missing_train_distributions(
    distributions_container::ride_distributions_container& ride_distributions,
    std::vector<std::vector<rating::connection_element>> const& trains,
    context const&);

namespace detail {
void compute_distributions_for_a_ride(
    unsigned int const light_connection_idx, node const& last_route_node,
    context const&, distributions_container::ride_distributions_container&
                        ride_distributions_container);

}  // namespace detail
}  // namespace ride_distribution
}  // namespace distributions_calculator
}  // namespace reliability
}  // namespace motis
