#include "motis/reliability/computation/calc_departure_distribution.h"

#include <cassert>
#include <iostream>

#include "motis/reliability/computation/data_departure.h"

namespace motis {
namespace reliability {
namespace calc_departure_distribution {

namespace detail {

/* helper for departure_at_scheduled_time */
probability train_arrived(data_departure const& data) {
  if (data.is_first_route_node_) {
    return data.train_info_.first_departure_distribution_
        ->probability_smaller_equal(0);
  }

  int const delay = timestamp_to_delay(
      data.train_info_.preceding_arrival_info_.scheduled_arrival_time_,
      data.scheduled_departure_time_ -
          data.train_info_.preceding_arrival_info_.min_standing_);
  return data.train_info_.preceding_arrival_info_.arrival_distribution_
      ->probability_smaller_equal(delay);
}

/* helper for departure_within_waiting_interval and
 * departure_after_waiting_interval */
probability train_arrives_at_time(data_departure const& data,
                                  time const timestamp) {
  if (data.is_first_route_node_) {
    return data.train_info_.first_departure_distribution_->probability_equal(
        timestamp_to_delay(data.scheduled_departure_time_, timestamp));
  }

  auto const& arrival_info = data.train_info_.preceding_arrival_info_;

  // largest delay such that an interchange from the feeder
  // into the departing train is still feasible.
  int const train_delay =
      timestamp_to_delay(arrival_info.scheduled_arrival_time_,
                         timestamp - arrival_info.min_standing_);

  // get the probability that the train arrives with a delay of 'train_delay'
  return arrival_info.arrival_distribution_->probability_equal(train_delay);
}

/* helper for departure_within_waiting_interval and
 * departure_after_waiting_interval */
probability train_arrives_before_time(data_departure const& data,
                                      time const timestamp) {
  if (data.is_first_route_node_) {
    return data.train_info_.first_departure_distribution_->probability_smaller(
        timestamp_to_delay(data.scheduled_departure_time_, timestamp));
  }

  auto const& arrival_info = data.train_info_.preceding_arrival_info_;

  // largest delay such that an interchange from the feeder
  // into the departing train is still feasible.
  int const train_delay =
      timestamp_to_delay(arrival_info.scheduled_arrival_time_,
                         timestamp - arrival_info.min_standing_);

  // get the probability that the train arrives with a delay of 'train_delay'
  return arrival_info.arrival_distribution_->probability_smaller(train_delay);
}

probability departure_independent_from_feeders(
    std::vector<data_departure::feeder_info> const& feeders,
    time const timestamp) {
  probability prob_no_waiting = 1.0;

  for (auto const& feeder : feeders) {
    time const waiting_interval_begin = timestamp - feeder.transfer_time_;
    probability prob_no_waiting_feeder = 0.0;

    if (waiting_interval_begin >= feeder.latest_feasible_arrival_) {
      prob_no_waiting_feeder = 1.0;
    } else {
      probability const prob_waiting_feeder =
          feeder.distribution_.probability_smaller_equal(
              timestamp_to_delay(feeder.scheduled_arrival_time_,
                                 feeder.latest_feasible_arrival_)) -
          feeder.distribution_.probability_smaller_equal(timestamp_to_delay(
              feeder.scheduled_arrival_time_, waiting_interval_begin));
      prob_no_waiting_feeder = 1.0 - prob_waiting_feeder;
    }
    prob_no_waiting *= prob_no_waiting_feeder;
  }

  return prob_no_waiting;
}

probability had_to_wait_for_feeders(
    std::vector<data_departure::feeder_info> const& feeders,
    std::vector<probability_distribution> const& modified_feeders_distributions,
    time const timestamp) {
  assert(feeders.size() == modified_feeders_distributions.size());
  if (feeders.empty()) {
    return 0.0;
  }

  probability prob_max_feeders_delay1 = 1.0, prob_max_feeders_delay2 = 1.0;
  for (unsigned int i = 0; i < feeders.size(); i++) {
    auto const& feeder = feeders[i];
    auto const& modified_distribution = modified_feeders_distributions[i];
    int const feeder_delay = timestamp_to_delay(
        feeder.scheduled_arrival_time_, timestamp - feeder.transfer_time_);

    prob_max_feeders_delay1 *=
        (1.0 - modified_distribution.probability_greater(feeder_delay));
    prob_max_feeders_delay2 *=
        (1.0 - modified_distribution.probability_greater(feeder_delay - 1));
  }

  return prob_max_feeders_delay1 - prob_max_feeders_delay2;
}

void cut_minutes_after_latest_feasible_arrival(
    std::vector<data_departure::feeder_info> const& feeders,
    std::vector<probability_distribution>& minutes_up_to_lfa) {
  minutes_up_to_lfa.resize(feeders.size());

  for (unsigned int feeder_idx = 0; feeder_idx < feeders.size(); feeder_idx++) {
    auto const& feeder = feeders[feeder_idx];
    std::vector<probability> probabilities;

    assert(feeder.latest_feasible_arrival_ >= feeder.scheduled_arrival_time_);
    time lfa_minute =
        feeder.latest_feasible_arrival_ - feeder.scheduled_arrival_time_;

    for (int feeder_delay = feeder.distribution_.first_minute();
         feeder_delay <= feeder.distribution_.last_minute(); feeder_delay++) {
      if (feeder_delay > lfa_minute)
        probabilities.push_back(0.0);
      else
        probabilities.push_back(
            feeder.distribution_.probability_equal(feeder_delay));
    }

    minutes_up_to_lfa[feeder_idx].init(probabilities,
                                       feeder.distribution_.first_minute());
  }
}

/**
 * calculate the probability for a departure at the scheduled departure time.
 */
probability departure_at_scheduled_time(data_departure const& data) {
  return train_arrived(data) *
         departure_independent_from_feeders(data.feeders_,
                                            data.scheduled_departure_time_);
}

/**
 * Calculate the probability for a departure within the waiting interval
 * at the point in time stored in parameter 'timestamp'.
 * This function can be used to compute train distributions,
 * and cannot handle interchanges.
 *
 * @param modified_feeders_distributions: vector containing for each feeder
 *        those minutes of the arrival distribution which are not later than the
 *        latest feasible arrival time of the feeder.
 *        This modified distributions are determined by
 *        'cut_minutes_after_latest_feasible_arrival'.
 */
probability departure_within_waiting_interval(
    data_departure const& data,
    std::vector<probability_distribution> const& modified_feeders_distributions,
    time const timestamp) {
  // probability that the departure at the given time is because of the delay of
  // the train itself
  probability const prob_train_has_delay =
      train_arrives_at_time(data, timestamp) *
      departure_independent_from_feeders(data.feeders_, timestamp);

  // probability that the departure at the given time is because of waiting of
  // at least one feeder
  probability const prob_a_feeder_has_delay =
      train_arrives_before_time(data, timestamp) *
      had_to_wait_for_feeders(data.feeders_, modified_feeders_distributions,
                              timestamp);

  return prob_train_has_delay + prob_a_feeder_has_delay;
}

/**
 * Calculate the probability for a departure after the waiting interval
 * at the point in time stored in 'timestamp'.
 */
probability departure_after_waiting_interval(data_departure const& data,
                                             time const timestamp) {
  return train_arrives_at_time(data, timestamp);
}

}  // namespace detail

/** method responsible for calculating a departure distribution */
void compute_departure_distribution(
    data_departure const& data,
    probability_distribution& departure_distribution) {
  if (data.is_message_.received_) {
    unsigned int const delay = std::max(
        data.is_message_.current_time_ - data.scheduled_departure_time_, 0);
    departure_distribution.init_one_point(
        delay, data.is_first_route_node_
                   ? data.train_info_.first_departure_distribution_->sum()
                   : data.train_info_.preceding_arrival_info_
                         .arrival_distribution_->sum());
    return;
  }

  duration const largest_delay = data.largest_delay();
  std::vector<probability> probabilties(largest_delay + 1);

  std::vector<probability_distribution> modified_feeders_distributions;
  detail::cut_minutes_after_latest_feasible_arrival(
      data.feeders_, modified_feeders_distributions);

  for (duration delay = 0; delay <= largest_delay; delay++) {
    time const departure_time = data.scheduled_departure_time_ + delay;
    if (delay == 0) {
      probabilties[delay] = detail::departure_at_scheduled_time(data);
    } else if (delay > 0 && delay <= data.maximum_waiting_time_) {
      probabilties[delay] = detail::departure_within_waiting_interval(
          data, modified_feeders_distributions, departure_time);
    } else {
      probabilties[delay] =
          detail::departure_after_waiting_interval(data, departure_time);
    }
  }
  departure_distribution.init(probabilties, 0);

  assert(std::abs(departure_distribution.sum() -
                  (data.is_first_route_node_
                       ? 1.0
                       : data.train_info_.preceding_arrival_info_
                             .arrival_distribution_->sum())) < 0.00001);
  assert(smaller_equal(departure_distribution.sum(), 1.0));
}

}  // namespace distributions_calculator
}  // namespace reliability
}  // namespace motis
