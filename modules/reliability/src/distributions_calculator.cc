#include "motis/reliability/distributions_calculator.h"

#include <cassert>
#include <iostream>

#include "motis/reliability/pd_calc_data_arrival.h"
#include "motis/reliability/pd_calc_data_departure.h"

namespace motis {
namespace reliability {
namespace distributions_calculator {

/** method responsible for calculating a departure distribution */
void compute_departure_distribution(
    pd_calc_data_departure const& data,
    probability_distribution& departure_distribution) {
  std::vector<probability_distribution> modified_feeders_distributions;
  detail::cut_minutes_after_latest_feasible_arrival(
      data.feeders_, modified_feeders_distributions);

  std::vector<probability> probabilties;
  duration const largest_delay = data.largest_delay();

  for (duration delay = 0; delay <= largest_delay; delay++) {
    time const departure_time = data.scheduled_departure_time() + delay;
    probability departure_probability = 0.0;

    if (delay == 0) {
      departure_probability = detail::departure_at_scheduled_time(data);
    } else if (delay > 0 && delay <= data.maximum_waiting_time_) {
      departure_probability = detail::departure_within_waiting_interval(
          data, modified_feeders_distributions, departure_time);
    } else {
      departure_probability = detail::departure_after_waiting_interval(data, departure_time);
    }

    probabilties.push_back(departure_probability);
  }

  departure_distribution.init(probabilties, 0);

  assert(equal(departure_distribution.sum(), 1.0));
}

#if 0
void compute_arrival_distribution(
    pd_calc_data_arrival const& data,
    probability_distribution& arrival_distribution) {

  if (LOGGING) {
    log_str << "\n\n\n\n---------------------------------------\n\n"
            << "calculate ARRIVAL PD: " << std::endl;
    data->output(log_str);
  }

  // index-variable used to access probability arrays ("probs" and
  // "dep_dist")
  unsigned int probs_idx = 0;

  // store all probabilities of the departure-distribution in the probability
  // array "dep_dist"
  // in order to facilitate an efficient access to the probabilities
  probability* dep_dist = new probability[data->dep_dist->get_num_minutes()];
  for (int i = data->dep_dist->get_first_minute();
       i <= data->dep_dist->get_last_minute(); i++, probs_idx++)
    dep_dist[probs_idx] = data->dep_dist->get_prob_equal(i);

  // if the arr_dist is NULL create a new probability_distribution object
  if (arr_dist == NULL) arr_dist = new probability_distribution;

  int last_minute = 0;

  if (parameters::parameters->use_d_b_t_t_distributions_is_active()) {

    // calculate the first delay minute (the left bound) of the arrival
    // distribution
    first_minute = data->min_travel_delay + data->dep_dist->get_first_minute();
    // calculate the last delay minute (the right bound) of the arrival
    // distribution
    last_minute = data->max_travel_delay + data->dep_dist->get_last_minute();
    // calculate the number of delay minutes of the arrival distribution
    probs_num = (last_minute - first_minute) + 1;
    if (probs_num >= 1000) {
      cout << "\n\n_error probs_num is too large!\narr_data: " << std::endl;
      data->output(cout);
      cout << "\nfirst:" << first_minute << " last: " << last_minute
           << " num: " << probs_num << std::endl;
      cout << "dep_dist: " << data->dep_dist->get_first_minute() << " "
           << data->dep_dist->get_last_minute() << std::endl;
      assert(false);
    }
    // return an empty distribution if the width is smaller than or equal 0
    if (probs_num <= 0) {
      cout << "\n_warning(get_prob_dist_arr): empty "
              "distribution" << std::endl;
      arr_dist->set_to_one_point_dist(0, 0.0);
      return;
    }
  } else {

    time earliest_arr_time(data->sched_dep_time);
    earliest_arr_time.add_minutes(data->dep_dist->get_first_minute());
    earliest_arr_time.add_minutes(data->sched_travel_time +
                                  data->min_travel_delay);
    first_minute =
        earliest_arr_time.get_difference_in_minutes(data->sched_arr_time);

    time latest_arr_time(data->sched_dep_time);
    latest_arr_time.add_minutes(data->dep_dist->get_last_minute());
    latest_arr_time.add_minutes(data->sched_travel_time +
                                data->max_travel_delay);
    last_minute =
        latest_arr_time.get_difference_in_minutes(data->sched_arr_time);

    probs_num = (last_minute - first_minute) + 1;
    if (probs_num <= 0) {
      cout << "\n_warning(get_prob_dist_arr): empty "
              "distribution" << std::endl;
      arr_dist->set_to_one_point_dist(0, 0.0);

      cout << "\n\n\nfirst_minute: " << first_minute
           << " last_minute: " << last_minute << " probs_num: " << probs_num
           << std::endl;

      cout << "earliest_arr_time: " << earliest_arr_time.output()
           << ", latest_arr_time: " << latest_arr_time.output() << std::endl;

      cout << "data->min_travel_delay: " << data->min_travel_delay
           << " data->max_travel_delay: " << data->max_travel_delay
           << std::endl;

      cout << "dep_dist: lb=" << data->dep_dist->get_first_minute()
           << " rb=" << data->dep_dist->get_last_minute() << std::endl;

      cout << "arr-data:" << std::endl;
      data->output(cout);

      return;
    }
  }  // end of else of if use_d_b_t_t_distributions

  // variable storing the probability calculated
  // for the currently processed delay minute
  probability new_prob = 0.0;
  // variable storing the probability of the considered travel times
  probability prob_t_t = 0.0;

  // variable storing the sum of the calculated arrival probabilities
  probability sum_new_probs = 0.0;
  // variable storing the greatest calculated probability
  probability max_new_prob = 0.0;
  // variable storing the index of "max_new_prob" in the "probs" array
  int max_new_prob_idx = 0;

  // variable storing the currently considered travel-time delay
  // used to access the travel time distribution
  int travel_time_delay = 0;
  // variable storing the currently considered departure delay
  // used to access the departure distribution
  int dep_delay = 0;
  // travel time distribution
  const probability_distribution* travel_time_dist = NULL;

  // variable storing the width of the departure distribution
  unsigned int dep_num_minutes = data->dep_dist->get_num_minutes();
  // the first delay minute (left bound) of the departure distribution
  int dep_first_min = data->dep_dist->get_first_minute();
  // probs_idx used to index probs
  probs_idx = 0;

  // iterate over all possible delay minutes of the arrival distribution
  // and calculate the probability of occurence.
  // this step is such a "convolution" of the departure distribution
  // and the travel time distributions. for each arrival delay,
  // we sum up the probabilities of all departure delay and travel time
  // combinations which result in that arrival delay.
  for (int arr_delay = first_minute; arr_delay <= last_minute;
       arr_delay++, probs_idx++) {

    // set new_prob to 0
    new_prob = 0.0;

    // iterate over all delay minutes of the departure distribution
    // and sum up the probabilities of arriving with arrival delay "arr_delay"
    // depending on departures with departure delays stored in "dep_delay" and
    // travel time delays stored in "travel_time_delay".
    for (unsigned int dep_idx = 0; dep_idx < dep_num_minutes; dep_idx++) {

      // departure delay
      dep_delay = dep_first_min + dep_idx;

      // get the travel-time distribution corresponding to the departure delay
      // 'dep_delay'
      travel_time_dist = data->get_travel_time_dist(dep_delay);

      // calculate the required travel time delay so that
      // the train arrives with arrival delay "arr_delay"
      // when departing with departure delay "dep_delay"
      travel_time_delay = arr_delay - dep_delay;

      if (parameters::parameters->use_d_b_t_t_distributions_is_active()) {
        // if the calculated travel time delay is not contained
        // in the travel time distribution, continue
        if (travel_time_delay < travel_time_dist->get_first_minute())
          continue;  // TODO: break here, since a shorter travel-time does not
        // exist here
      } else {
        // break here, since a shorter travel-time does not exist here
        if (travel_time_delay < travel_time_dist->get_first_minute()) break;
      }

      // continue and ignore this departure delay,
      // since such a long travel time does not exist here
      if (travel_time_delay > travel_time_dist->get_last_minute()) continue;

      // get the probability of that the travel time
      // equals scheduled-travel-time + "travel_time_delay"
      prob_t_t = travel_time_dist->get_prob_equal(travel_time_delay);

      // sum up the probabilities of arriving with arrival delay "arr_delay"
      // depending on departures with departure delays stored in "dep_delay" and
      // travel time delays stored in "travel_time_delay".
      new_prob += (prob_t_t * dep_dist[dep_idx]);

      if (LOGGING && prob_t_t > 0.0 && dep_dist[dep_idx] > 0.0)
        log_str << "p(d=" << dep_delay << ",t=" << travel_time_delay
                << ") = " << dep_dist[dep_idx] << "*" << prob_t_t << "="
                << (dep_dist[dep_idx] * prob_t_t) << ", " << flush;

    }  // end of for dep

    // store the probability of the current arrival delay minute
    if (probs_idx >= 1000)
      cout << "\n_warning: probs_idx too large" << std::endl;
    assert(probs_idx < 1000);
    probs[probs_idx] = new_prob;

    if (parameters::parameters->use_d_b_t_t_distributions_is_active()) {
      // sum up all probabilities of the arrival distribution
      sum_new_probs += new_prob;
      // find the maximum probability of the distribution
      if (new_prob > max_new_prob) {
        max_new_prob = new_prob;
        max_new_prob_idx = probs_idx;
      }
    }

    if (LOGGING && new_prob > 0.0)
      log_str << "\n_p_arr(" << arr_delay << "): " << new_prob
              << "\n\n------------------" << std::endl;
  }

  if (parameters::parameters->use_d_b_t_t_distributions_is_active()) {
    // because of rounding errors the sum of the calculated arrival distribution
    // could get smaller than the sum of the preceding departure distribution.
    // this also depends on inaccuracies of travel time distributions.
    // in this case we correct the arrival distribution by adding this error
    // value
    // to the greatest probability (peak) of the arrival distribution
    probability error = data->dep_dist->get_sum() - sum_new_probs;
    if (error > 0.01) {
      cout << "\n_warning(get_prob_dist_arr): too great "
              "error: " << error << std::endl;
    }
    if (max_new_prob_idx >= 1000)
      cout << "\n_warning: max_new_prob_idx too large" << std::endl;
    assert(max_new_prob_idx < 1000);
    probs[max_new_prob_idx] += error;
  }

  // initialize the arrival distribution using
  // the calculated probabilities in the probability array "probs"
  arr_dist->init(probs, first_minute, probs_num, ignore_small_values);

  if (LOGGING) {
    log_str << "\n_arr-dist:" << std::endl;
    arr_dist->output(
        log_str, data->sched_arr_time.get_minutes_after_reference_time(), true);
  }

#ifdef CHECK_DISTS_SUM
  probability sum1 = arr_dist->get_sum();
  if (sum1 < 0.99 || sum1 > 1.0001) {
    cout << "\n\n_arr-data:" << std::endl;
    data->output(cout);
    cout << "\n\n_arr-dist: " << std::endl;
    arr_dist->output(
        cout, data->sched_arr_time.get_minutes_after_reference_time(), true);
    cout << "\nfirst_minute: " << first_minute
         << " last_minute: " << last_minute << " (" << probs[last_minute] << ")"
         << std::endl;
    getchar();
  }
#endif

  // delete the probability array "dep_dist"
  delete[] dep_dist;
}
#endif

namespace detail {

/* helper for departure_at_scheduled_time */
probability train_early_enough(pd_calc_data_departure const& data) {
  if (data.is_first_route_node_) {
    return data.train_info_.first_departure_distribution
        ->probability_smaller_equal(0);
  }

  int const delay = timestamp_to_delay(
      data.train_info_.preceding_arrival_info_.arrival_time_,
      data.scheduled_departure_time() -
          data.train_info_.preceding_arrival_info_.min_standing_);
  return data.train_info_.preceding_arrival_info_.arrival_distribution_
      ->probability_smaller_equal(delay);
}

probability departure_at_scheduled_time(pd_calc_data_departure const& data) {
  probability result = train_early_enough(data) *
                       departure_independent_from_feeders(
                           data.feeders_, data.scheduled_departure_time());

#if 0
  // if there is an interchange before this departure
  // consider the effect of the feeder involved in the interchange.
  if (data->has_interchange()) {

    // index of the feeder involved in the interchange
    int feeder_idx = data->get_i_c_feeder_idx();

    // latest arrival-time of the feeder-train so that the departure
    // is possible at the scheduled time
    time latest_arr_time_i_c_feeder(data->sched_dep_time);
    latest_arr_time_i_c_feeder.subtract_minutes(
        data->transfer_times[feeder_idx]);

    // probability that there is no need to wait for the feeder involved in
    // the interchange in the connection before this departure event
    probability ic_feeder_early_enough =
        data->feeders[feeder_idx]->get_prob_smaller_equal(
            latest_arr_time_i_c_feeder.get_difference_in_minutes(
                data->arr_time_feeders[feeder_idx]));

    result *= ic_feeder_early_enough;

    if (LOGGING)
      log_str << "ic_feeder_early_enough=" << ic_feeder_early_enough
              << std::endl;
  }
#endif

  return result;
}

/* helper for departure_within_waiting_interval and
 * departure_after_waiting_interval */
probability train_arrives_at_time(pd_calc_data_departure const& data,
                                  time const timestamp) {
  if (data.is_first_route_node_) {
    return data.train_info_.first_departure_distribution->probability_equal(
        timestamp_to_delay(data.scheduled_departure_time(), timestamp));
  }

  auto const& arrival_info = data.train_info_.preceding_arrival_info_;

  // largest delay such that an interchange from the feeder
  // into the departing train is still feasible.
  int const train_delay = timestamp_to_delay(
      arrival_info.arrival_time_, timestamp - arrival_info.min_standing_);

  // get the probability that the train arrives with a delay of 'train_delay'
  return arrival_info.arrival_distribution_->probability_equal(train_delay);
}

/* helper for departure_within_waiting_interval and
 * departure_after_waiting_interval */
probability train_arrives_before_time(pd_calc_data_departure const& data,
                                      time const timestamp) {
  if (data.is_first_route_node_) {
    return data.train_info_.first_departure_distribution->probability_smaller(
        timestamp_to_delay(data.scheduled_departure_time(), timestamp));
  }

  auto const& arrival_info = data.train_info_.preceding_arrival_info_;

  // largest delay such that an interchange from the feeder
  // into the departing train is still feasible.
  int const train_delay = timestamp_to_delay(
      arrival_info.arrival_time_, timestamp - arrival_info.min_standing_);

  // get the probability that the train arrives with a delay of 'train_delay'
  return arrival_info.arrival_distribution_->probability_smaller(train_delay);
}

probability departure_within_waiting_interval(
    pd_calc_data_departure const& data,
    std::vector<probability_distribution> const& modified_feeders_distributions,
    time const timestamp) {
  // probability that there is no need to wait for a feeder train
  probability const prob_no_waiting_for_feeders =
      departure_independent_from_feeders(data.feeders_, timestamp);

  // probability that the departure at the given time is because of the delay of
  // the train itself
  probability const prob_train_has_delay =
      train_arrives_at_time(data, timestamp) * prob_no_waiting_for_feeders;

  // probability that the departure at the given time is because of waiting of
  // at least one feeder
  probability const prob_a_feeder_has_delay =
      train_arrives_before_time(data, timestamp) *
      had_to_wait_for_feeders(data.feeders_, modified_feeders_distributions,
                              timestamp);

  return prob_train_has_delay + prob_a_feeder_has_delay;
}

probability departure_after_waiting_interval(pd_calc_data_departure const& data,
                                             time const timestamp) {
  probability result = train_arrives_at_time(data, timestamp);

#if 0
  // if there is an interchange before this departure
  // consider the effect of the feeder involved in the interchange.
  if (data->has_interchange()) {
    // index of the feeder involved in the interchange
    int feeder_idx = data->get_i_c_feeder_idx();

    // latest arrival-time of the feeder so that a departure at the given
    // timestamp
    // is possible
    time latest_arr_time_i_c_feeder(timestamp);
    latest_arr_time_i_c_feeder.subtract_minutes(
        data->transfer_times[feeder_idx]);
    int delay = latest_arr_time_i_c_feeder.get_difference_in_minutes(
        data->arr_time_feeders[feeder_idx]);

    // get the probabilty that the feeder has not a delay greater than 'delay'
    // so that the train has not to wait for this feeder for a longer timestamp
    probability prob_no_waiting_for_i_c_feeder =
        data->feeders[feeder_idx]->get_prob_smaller_equal(delay);

    // prob_res has to be multiplied with prob_no_waiting_for_i_c_feeder
    result *= prob_no_waiting_for_i_c_feeder;

    if (LOGGING)
      log_str << "prob_no_wait_for_i_c_feeder: "
              << prob_no_waiting_for_i_c_feeder << std::endl;
  }
#endif

  return result;
}

#if 0

/**
 * calculate the probability for a departure in the waiting interval
 * at a point in time stored in parameter 'timestamp'. this method is called
 * when there is an interchange before this departure event.
 *
 * @param pd_calc_data_departure* object containing all necessary data
 * @param const time& point in time
 * @param probability_distribution** arr_dists_only_feasible_part corresponding
 *modified arrival distributions of the feeders
 *                           (see comments of method init_arr_dists_y)
 * @return probability probability
 */
probability get_prob_dep_waiting_interval_interchange(
    pd_calc_data_departure* data, const time& timestamp,
    probability_distribution** arr_dists_only_feasible_part) {

  probability prob_res = 0.0;

  /* probabilities of the departing train */

  // latest arrival-time of the train so that a departure at the given time is
  // possible
  time latest_arr_time_train(timestamp);
  latest_arr_time_train.subtract_minutes(data->min_standing);

  // probability that the train arrives at the given timestamp
  probability prob_train_arr_at_time = 0.0;
  // probability that the train arrives before the given timestamp
  probability prob_train_arr_before_time = 1.0;
  if (data->arr_dist != NULL) {
    int delay_train =
        latest_arr_time_train.get_difference_in_minutes(data->arr_time_train);
    prob_train_arr_at_time = data->arr_dist->get_prob_equal(delay_train);
    prob_train_arr_before_time = data->arr_dist->get_prob_smaller(delay_train);
  }

  /* probabilties of the feeders */

  // probability that there is no need to wait for a feeder train
  probability prob_no_waiting_for_feeders =
      departure_independent_from_feeders(data, timestamp);

  // probability that the train has to wait for at least one feeder
  probability prob_waiting_for_feeders = get_prob_waiting_for_feeders(
      data, timestamp, arr_dists_only_feasible_part);

  /* probabilities of the interchange-feeder */

  // index of the feeder involved in the interchange
  int feeder_idx = data->get_i_c_feeder_idx();

  // latest arrival-time of the feeder so that a departure at the given time is
  // possible
  time latest_arr_time_i_c_feeder(timestamp);
  latest_arr_time_i_c_feeder.subtract_minutes(data->transfer_times[feeder_idx]);
  int delay_i_c_feeder = latest_arr_time_i_c_feeder.get_difference_in_minutes(
      data->arr_time_feeders[feeder_idx]);

  // probability that the feeder arrives at the timestamp
  // 'latest_arr_time_i_c_feeder'
  probability prob_i_c_feeder_a_t_time =
      data->feeders[feeder_idx]->get_prob_equal(delay_i_c_feeder);
  // probability that the feeder arrives before the timestamp
  // 'latest_arr_time_i_c_feeder'
  probability prob_i_c_feeder_not_later_than_time =
      data->feeders[feeder_idx]->get_prob_smaller_equal(delay_i_c_feeder);

  /* probability that the train waits */

  // probability that the train waits of a feeder
  // plus the probability if the train waits for the ic-feeder.
  // in the second term we have to deduct the probability of waiting for any
  // feeder
  // since this case is already considered in the first summand of
  // 'prob_waiting'
  probability prob_waiting =
      prob_waiting_for_feeders * prob_i_c_feeder_not_later_than_time;
  if (latest_arr_time_i_c_feeder <=
      data->latest_feasible_arr_feeders[feeder_idx])
    prob_waiting += (prob_i_c_feeder_a_t_time *
                     (prob_no_waiting_for_feeders - prob_waiting_for_feeders));

  /* summands of the departure-probability */

  // probability that the departure at the given time is because of the delay of
  // the train itself
  probability prob_train_has_delay = prob_train_arr_at_time *
                                     prob_i_c_feeder_not_later_than_time *
                                     prob_no_waiting_for_feeders;

  // probability that the train waits
  probability prob_train_waits = prob_train_arr_before_time * prob_waiting;

  // departure probability
  prob_res = (prob_train_has_delay + prob_train_waits);

  if (LOGGING)
    log_str << "get_prob_dist_dep_waiting_interval(" << format_time(timestamp)
            << "): " << std::endl
            << "prob_train_has_delay = " << prob_train_has_delay
            << ", prob_train_waits = " << prob_train_waits << std::endl
            << "prob_train_arr_at_time(" << latest_arr_time_train.output()
            << ") = " << prob_train_arr_at_time
            << ", prob_train_arr_before_time = " << prob_train_arr_before_time
            << std::endl
            << "prob_no_waiting_for_feeders = " << prob_no_waiting_for_feeders
            << ", prob_waiting_for_feeders = " << prob_waiting_for_feeders
            << std::endl
            << "prob_i_c_feeder_a_t_time("
            << latest_arr_time_i_c_feeder.output()
            << ") = " << prob_i_c_feeder_a_t_time
            << ", prob_i_c_feeder_not_later_than_time = "
            << prob_i_c_feeder_not_later_than_time << std::endl
            << "prob_waiting = " << prob_waiting
            << ", latest_feasible_arr_i_c_feeder = "
            << data->latest_feasible_arr_feeders[feeder_idx].output()
            << ", difference = "
            << (prob_no_waiting_for_feeders - prob_waiting_for_feeders)
            << std::endl;

  if (prob_res < 0.0)
    cout << "\n\n_warning(get_prob_dist_dep_waiting_"
            "interval_interchange): negative probability" << std::endl;

  return prob_res;
}

#endif

probability departure_independent_from_feeders(
    std::vector<pd_calc_data_departure::feeder_info> const& feeders,
    time const timestamp) {
  probability prob_no_waiting = 1.0;

  for (auto const& feeder : feeders) {
    time const waiting_interval_begin = timestamp - feeder.transfer_time_;
    probability prob_no_waiting_feeder = 0.0;

    if (waiting_interval_begin >= feeder.latest_feasible_arrival_) {
      prob_no_waiting_feeder = 1.0;
    } else {
      probability const prob_waiting_feeder =
          feeder.distribution_.probability_smaller_equal(timestamp_to_delay(
              feeder.arrival_time_, feeder.latest_feasible_arrival_)) -
          feeder.distribution_.probability_smaller_equal(
              timestamp_to_delay(feeder.arrival_time_, waiting_interval_begin));

      prob_no_waiting_feeder = 1.0 - prob_waiting_feeder;
    }
    prob_no_waiting *= prob_no_waiting_feeder;
  }

  return prob_no_waiting;
}

probability had_to_wait_for_feeders(
    std::vector<pd_calc_data_departure::feeder_info> const& feeders,
    std::vector<probability_distribution> const& modified_feeders_distributions,
    time const timestamp) {
  assert(feeders.size() == modified_feeders_distributions.size());

  if (feeders.size() == 0) {
    return 0.0;
  }

  probability prob_max_feeders_delay1 = 1.0, prob_max_feeders_delay2 = 1.0;

  for (unsigned int i = 0; i < feeders.size(); i++) {
    auto const& feeder = feeders[i];
    auto const& modified_distribution = modified_feeders_distributions[i];

    int const feeder_delay = timestamp_to_delay(
        feeder.arrival_time_, timestamp - feeder.transfer_time_);

    prob_max_feeders_delay1 *=
        (1.0 - modified_distribution.probability_greater(feeder_delay));

    prob_max_feeders_delay2 *=
        (1.0 - modified_distribution.probability_greater(feeder_delay - 1));
  }

  return prob_max_feeders_delay1 - prob_max_feeders_delay2;
}

void cut_minutes_after_latest_feasible_arrival(
    std::vector<pd_calc_data_departure::feeder_info> const& feeders,
    std::vector<probability_distribution>& minutes_up_to_lfa) {
  minutes_up_to_lfa.resize(feeders.size());

  for (unsigned int feeder_idx = 0; feeder_idx < feeders.size(); feeder_idx++) {
    auto const& feeder = feeders[feeder_idx];
    std::vector<probability> probabilities;

    assert(feeder.latest_feasible_arrival_ >= feeder.arrival_time_);
    time lfa_minute = feeder.latest_feasible_arrival_ - feeder.arrival_time_;

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

}  // namespace detail
}  // namespace distributions_calculator
}  // namespace reliability
}  // namespace motis
