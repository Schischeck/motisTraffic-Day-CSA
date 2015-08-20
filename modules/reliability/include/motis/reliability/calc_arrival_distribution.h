#pragma once

#include <functional>
#include <vector>

#include "motis/core/schedule/time.h"

#include "motis/reliability/probability_distribution.h"

namespace motis {
namespace reliability {

struct pd_calc_data_arrival;

namespace calc_arrival_distribution {

void compute_arrival_distribution(
    pd_calc_data_arrival const& data,
    probability_distribution& arrival_distribution);

namespace detail {
using travel_distributions_type =
    std::vector<std::reference_wrapper<probability_distribution const> >;

/**
 * For each delay minute of the departure distribution,
 * the vector 'travel_distributions' is initialized
 * with the corresponding travel time distribution.
 * Entry travel_distributions[i] contains the travel-time-distribution
 * for departure delay i.
 */
void init_travel_distributions(pd_calc_data_arrival const& data,
                               travel_distributions_type& travel_distributions,
                               int& first_minute, int& last_minute);

/**
 * Because of rounding errors, the sum of the calculated arrival distribution
 * could get smaller than the sum of the preceding departure distribution.
 * This also depends on inaccuracies of the travel time distributions.
 * In this case, we correct the arrival distribution by adding the error
 * value to the greatest probability (peak) of the arrival distribution
 */
void correct_rounding_errors(probability const& expected_sum,
                             std::vector<probability>& computed_probabilities);

}  // namespace detail

}  // namespace calc_arrival_distribution

}  // namespace reliability
}  // namespace motis
