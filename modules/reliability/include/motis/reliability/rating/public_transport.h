#pragma once

#include <vector>

namespace motis {
struct schedule;
namespace reliability {
struct probability_distribution;
struct start_and_travel_distributions;
namespace distributions_calculator {
namespace common {
struct queue_element;
}
}
namespace distributions_container {
struct precomputed_distributions_container;
}

namespace rating {
namespace public_transport {

std::vector<probability_distribution> const rate(
    std::vector<
        std::vector<distributions_calculator::common::queue_element>> const&,
    schedule const&,
    distributions_container::precomputed_distributions_container const&,
    start_and_travel_distributions const&);

}  // namespace public_transport
}  // namespace rating
}  // namespace reliability
}  // namespace motis
