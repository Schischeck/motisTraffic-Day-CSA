#include "motis/reliability/computation/calc_departure_distribution_interchange.h"

#include <cassert>
#include <iostream>

#include "motis/reliability/computation/calc_departure_distribution.h"
#include "motis/reliability/computation/data_departure_interchange.h"

namespace motis {
namespace reliability {
namespace calc_departure_distribution {
namespace interchange {

namespace detail {

probability ic_feeder_arrived(
    data_departure_interchange::interchange_feeder_info const&
        interchange_feeder_info,
    time const timestamp) {
  time const latest_arrival_for_valid_interchange =
      timestamp - interchange_feeder_info.transfer_time_;

  return interchange_feeder_info.arrival_distribution_
      ->probability_smaller_equal(
          (int)(latest_arrival_for_valid_interchange -
                interchange_feeder_info.scheduled_arrival_time_));
}

probability ic_feeder_arrives_at_time(
    data_departure_interchange::interchange_feeder_info const&
        interchange_feeder_info,
    time const timestamp) {
  time const latest_arrival_for_valid_interchange =
      timestamp - interchange_feeder_info.transfer_time_;

  return interchange_feeder_info.arrival_distribution_->probability_equal(
      (int)(latest_arrival_for_valid_interchange -
            interchange_feeder_info.scheduled_arrival_time_));
}

probability departure_at_scheduled_time(
    data_departure_interchange const& data) {
  return ic_feeder_arrived(data.interchange_feeder_info_,
                           data.scheduled_departure_time_) *
         calc_departure_distribution::detail::train_arrived(data) *
         calc_departure_distribution::detail::
             departure_independent_from_feeders(data.feeders_,
                                                data.scheduled_departure_time_);
}

probability departure_within_waiting_interval(
    data_departure_interchange const& data,
    std::vector<probability_distribution> const& modified_feeders_distributions,
    time const timestamp) {
  probability const ic_feeder_arrived =
      detail::ic_feeder_arrived(data.interchange_feeder_info_, timestamp);
  probability const ic_feeder_arrives_at_time =
      detail::ic_feeder_arrives_at_time(data.interchange_feeder_info_,
                                        timestamp);
  probability const departure_independent_from_feeders =
      calc_departure_distribution::detail::departure_independent_from_feeders(
          data.feeders_, timestamp);
  probability const had_to_wait_for_feeders =
      calc_departure_distribution::detail::had_to_wait_for_feeders(
          data.feeders_, modified_feeders_distributions, timestamp);
  probability const train_arrives_at_time =
      calc_departure_distribution::detail::train_arrives_at_time(data,
                                                                 timestamp);
  probability const train_arrives_before_time =
      calc_departure_distribution::detail::train_arrives_before_time(data,
                                                                     timestamp);
  probability prob_waiting = had_to_wait_for_feeders * ic_feeder_arrived;
  if (timestamp - data.interchange_feeder_info_.transfer_time_ <=
      data.interchange_feeder_info_.latest_feasible_arrival_) {
    prob_waiting +=
        (ic_feeder_arrives_at_time *
         (departure_independent_from_feeders - had_to_wait_for_feeders));
  }

  probability const prob_train_has_delay = train_arrives_at_time *
                                           departure_independent_from_feeders *
                                           ic_feeder_arrived;
  probability const prob_train_waits = train_arrives_before_time * prob_waiting;

  return prob_train_has_delay + prob_train_waits;
}

probability departure_after_waiting_interval(
    data_departure_interchange const& data, time const timestamp) {
  return ic_feeder_arrived(data.interchange_feeder_info_, timestamp) *
         calc_departure_distribution::detail::train_arrives_at_time(data,
                                                                    timestamp);
}

}  // namespace detail

/** method responsible for calculating a departure distribution */
void compute_departure_distribution(
    data_departure_interchange const& data,
    probability_distribution& departure_distribution) {
  if (data.is_message_.received_) {
    unsigned int const delay =
        data.is_message_.current_time_ > data.scheduled_departure_time_
            ? data.is_message_.current_time_ - data.scheduled_departure_time_
            : 0;
    departure_distribution.init_one_point(
        delay, data.interchange_feeder_info_.arrival_distribution_->sum());
    return;
  }

  duration const largest_delay = data.largest_delay();
  std::vector<probability> probabilties(largest_delay + 1);

  std::vector<probability_distribution> modified_feeders_distributions;
  calc_departure_distribution::detail::
      cut_minutes_after_latest_feasible_arrival(data.feeders_,
                                                modified_feeders_distributions);

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

  assert(smaller_equal(
      departure_distribution.sum(),
      data.interchange_feeder_info_.arrival_distribution_->sum()));
}

}  // namespace interchange
}  // namespace calc_departure_distribution
}  // namespace reliability
}  // namespace motis
