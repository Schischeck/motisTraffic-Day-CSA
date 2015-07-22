#pragma once

#include <vector>

#include "motis/core/schedule/time.h"

#include "motis/reliability/pd_calc_data_departure.h"
#include "motis/reliability/probability_distribution.h"

namespace motis {
namespace reliability {

struct pd_calc_data_arrival;

namespace distributions_calculator {

void compute_departure_distribution(
    pd_calc_data_departure const& data,
    probability_distribution& departure_distribution);

void compute_arrival_distribution(
    pd_calc_data_arrival const& data,
    probability_distribution& arrival_distribution);

namespace detail {

/**
 * Calculate the probability that from the time stored in 'timestamp'
 * the departure of the train does not depend on any feeder.
 */
probability departure_independent_from_feeders(
    std::vector<pd_calc_data_departure::feeder_info> const& feeders,
    time const timestamp);
}
};

}  // namespace reliability
}  // namespace motis
