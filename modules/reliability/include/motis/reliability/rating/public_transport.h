#pragma once

#include <vector>

namespace motis {
struct schedule;
namespace reliability {
struct context;
struct probability_distribution;
struct start_and_travel_distributions;
namespace distributions_container {
struct precomputed_distributions_container;
}

namespace rating {
struct connection_element;
struct rating_element;
namespace public_transport {
void rate(std::vector<rating_element>& ratings,
          std::vector<std::vector<connection_element>> const&,
          bool const first_element_already_processed, context const&);

}  // namespace public_transport
}  // namespace rating
}  // namespace reliability
}  // namespace motis
