#pragma once

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
