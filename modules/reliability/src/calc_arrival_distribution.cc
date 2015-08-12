#include <cassert>

#include "motis/reliability/calc_arrival_distribution.h"
#include "motis/reliability/pd_calc_data_arrival.h"
#include "motis/reliability/tt_distributions_manager.h"

namespace motis {
namespace reliability {
namespace calc_arrival_distribution {

void compute_arrival_distribution(
    pd_calc_data_arrival const& data,
    probability_distribution& arrival_distribution) {

  std::vector<probability> computed_probabilities;

  // Efficient access to the probabilities of the departure distribution
  std::vector<probability> dep_dist;
  data.departure_info_.distribution_->get_probabilities(dep_dist);

  int const first_minute = data.departure_info_.distribution_->first_minute() +
                           data.travel_time_info_->min_travel_delay_;
  int const last_minute = data.departure_info_.distribution_->last_minute() +
                          data.travel_time_info_->max_travel_delay_;

  // This step is a "convolution" of the departure distribution
  // with the travel time distributions. For each arrival delay,
  // we sum up the probabilities of all departure delay and travel time
  // combinations that result in that arrival delay.
  for (int arr_delay = first_minute; arr_delay <= last_minute; arr_delay++) {

    probability computed_probability = 0.0;

    for (unsigned int dep_prob_idx = 0; dep_prob_idx < dep_dist.size();
         dep_prob_idx++) {
      unsigned int const dep_delay =
          (unsigned int)data.departure_info_.distribution_->first_minute() +
          dep_prob_idx;
      int const travel_time_delay = arr_delay - dep_delay;
      auto const& travel_time_dist =
          data.travel_time_info_->get_travel_time_distribution(dep_delay);

      if (travel_time_delay < travel_time_dist.first_minute() ||
          travel_time_delay > travel_time_dist.last_minute()) {
        continue;
      }

      // sum up the probabilities of arriving with arrival delay "arr_delay"
      // depending on departures with departure delays stored in "dep_delay" and
      // travel time delays stored in "travel_time_delay".
      computed_probability +=
          (travel_time_dist.probability_equal(travel_time_delay) *
           dep_dist[dep_prob_idx]);

    }  // end of for dep_prob_idx

    computed_probabilities.push_back(computed_probability);
  }  // end of for arr_delay

  detail::correct_rounding_errors(data.departure_info_.distribution_->sum(),
                                  computed_probabilities);

  arrival_distribution.init(computed_probabilities, first_minute);
}

namespace detail {

#include <iostream>
void correct_rounding_errors(probability const& expected_sum,
                             std::vector<probability>& computed_probabilities) {
  probability sum = 0.0;
  int index_max = 0;

  // compute sum and find max
  for (unsigned int i = 0; i < computed_probabilities.size(); i++) {
    sum += computed_probabilities[i];
    if (computed_probabilities[i] > computed_probabilities[index_max]) {
      index_max = i;
    }
  }

  probability const difference = expected_sum - sum;

  assert(!greater(sum, expected_sum));
  assert(!greater(difference, 0.0001));

  computed_probabilities[index_max] += difference;
}
}  // namespace detail
}  // namespace calc_arrival_distibution
}  // namespace reliability
}  // namespace motis
