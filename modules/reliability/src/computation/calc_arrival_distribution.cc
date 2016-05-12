#include "motis/reliability/computation/calc_arrival_distribution.h"

#include <cassert>
#include <algorithm>

#include "motis/reliability/computation/data_arrival.h"
#include "motis/reliability/distributions/start_and_travel_distributions.h"

namespace motis {
namespace reliability {
namespace calc_arrival_distribution {

void compute_arrival_distribution(
    data_arrival const& data, probability_distribution& arrival_distribution) {
  if (data.is_message_.received_) {
    arrival_distribution.init_one_point(
        data.is_message_.current_time_ - data.scheduled_arrival_time_,
        data.departure_info_.distribution_.sum());
    return;
  }
  // if there is no travel distribution for this class,
  // copy this arrival distribution
  if (data.travel_distributions_.empty()) {
    arrival_distribution.init(data.departure_info_.distribution_);
    return;
  }

  // Efficient access to the probabilities of the departure distribution
  std::vector<probability> dep_dist;
  data.departure_info_.distribution_.get_probabilities(dep_dist);

  std::vector<probability> computed_probabilities(
      (data.right_bound_ - data.left_bound_) + 1);
  // This step is a "convolution" of the departure distribution
  // with the travel time distributions. For each arrival delay,
  // we sum up the probabilities of all departure delay and travel time
  // combinations that result in that arrival delay.
  for (int arr_delay = data.left_bound_; arr_delay <= data.right_bound_;
       arr_delay++) {
    probability computed_probability = 0.0;
    for (unsigned dep_prob_idx = 0; dep_prob_idx < dep_dist.size();
         dep_prob_idx++) {
      assert(data.departure_info_.distribution_.first_minute() >= 0);
      auto const dep_delay =
          static_cast<unsigned>(
              data.departure_info_.distribution_.first_minute()) +
          dep_prob_idx;
      int const travel_time_delay = arr_delay - dep_delay;
      auto const& travel_time_dist =
          data.travel_distributions_[dep_delay].get();

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
    assert(computed_probability >= 0.0);
    computed_probabilities[arr_delay - data.left_bound_] = computed_probability;
  }  // end of for arr_delay

  detail::correct_rounding_errors(data.departure_info_.distribution_.sum(),
                                  computed_probabilities);

  arrival_distribution.init(computed_probabilities, data.left_bound_);
}

namespace detail {

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

  if (!equal(difference, 0.0)) {
    assert(smaller_equal(sum + difference, expected_sum));
    assert(smaller_equal(std::abs(difference), 0.0001));

    computed_probabilities[index_max] += difference;

    assert(computed_probabilities[index_max] >= 0.0);
  }
}
}  // namespace detail
}  // namespace calc_arrival_distribution
}  // namespace reliability
}  // namespace motis
