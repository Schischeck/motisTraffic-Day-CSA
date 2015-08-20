#include <algorithm>
#include <cassert>

#include "motis/reliability/calc_arrival_distribution.h"

#include "motis/reliability/start_and_travel_distributions.h"
#include "motis/reliability/pd_calc_data_arrival.h"

namespace motis {
namespace reliability {
namespace calc_arrival_distribution {

void compute_arrival_distribution(
    pd_calc_data_arrival const& data,
    probability_distribution& arrival_distribution) {
  // Efficient access to the probabilities of the departure distribution
  std::vector<probability> dep_dist;
  data.departure_info_.distribution_->get_probabilities(dep_dist);

  // for each departure delay get the corresponding travel time distribution
  int first_minute, last_minute;
  detail::travel_distributions_type travel_distributions;
  detail::init_travel_distributions(data, travel_distributions, first_minute,
                                    last_minute);

  std::vector<probability> computed_probabilities((last_minute - first_minute) +
                                                  1);
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
      auto const& travel_time_dist = travel_distributions[dep_delay];

      if (travel_time_delay < travel_time_dist.get().first_minute() ||
          travel_time_delay > travel_time_dist.get().last_minute()) {
        continue;
      }

      // sum up the probabilities of arriving with arrival delay "arr_delay"
      // depending on departures with departure delays stored in "dep_delay" and
      // travel time delays stored in "travel_time_delay".
      computed_probability +=
          (travel_time_dist.get().probability_equal(travel_time_delay) *
           dep_dist[dep_prob_idx]);

    }  // end of for dep_prob_idx
    computed_probabilities[arr_delay - first_minute] = computed_probability;
  }  // end of for arr_delay

  detail::correct_rounding_errors(data.departure_info_.distribution_->sum(),
                                  computed_probabilities);

  arrival_distribution.init(computed_probabilities, first_minute);
}

namespace detail {

inline void add_travel_distribution(
    int const dep_delay, probability_distribution const& travel_distribution,
    travel_distributions_type& travel_distributions, int& first_minute,
    int& last_minute) {
  travel_distributions.push_back(travel_distribution);
  if (dep_delay + travel_distribution.first_minute() < first_minute) {
    first_minute = dep_delay + travel_distribution.first_minute();
  }
  if (dep_delay + travel_distribution.last_minute() > last_minute) {
    last_minute = dep_delay + travel_distribution.last_minute();
  }
}

inline void init_travel_distributions(
    pd_calc_data_arrival const& data,
    travel_distributions_type& travel_distributions, int& first_minute,
    int& last_minute) {
  assert(data.travel_distributions_.size() > 0);
  assert(data.travel_distributions_[0].departure_delay_from_ == 0);

  first_minute = data.departure_info_.distribution_->first_minute() +
                 data.travel_distributions_[0].distribution_.first_minute();
  last_minute = data.departure_info_.distribution_->first_minute() +
                data.travel_distributions_[0].distribution_.last_minute();

  // for each delay minute of the departure distribution
  // store the corresponding travel distribution into the
  // vector 'travel_distributions'
  for (auto const& travel_dist : data.travel_distributions_) {
    for (int dep_delay = travel_dist.departure_delay_from_;
         dep_delay <= travel_dist.departure_delay_to_ &&
             dep_delay <= data.departure_info_.distribution_->last_minute();
         dep_delay++) {
      add_travel_distribution(dep_delay, travel_dist.distribution_,
                              travel_distributions, first_minute, last_minute);
    }
  }
  // if the departure distribution has more minutes than
  // the largest delay expected by data.travel_distributions_
  // use the last distribution contained in data.travel_distributions_
  // for those delay minutes.
  for (int d = travel_distributions.size();
       d <= data.departure_info_.distribution_->last_minute(); d++) {
    add_travel_distribution(
        d, data.travel_distributions_[data.travel_distributions_.size() - 1]
               .distribution_,
        travel_distributions, first_minute, last_minute);
  }

  assert(travel_distributions.size() ==
         data.departure_info_.distribution_->last_minute() + 1);
}

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
