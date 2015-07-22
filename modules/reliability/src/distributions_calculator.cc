#include "motis/reliability/distributions_calculator.h"

#include <cassert>
#include <iostream>

#include "motis/reliability/pd_calc_data_arrival.h"
#include "motis/reliability/pd_calc_data_departure.h"

#define MAX_DIFF_VALUES 0.005

#define LOGGING false
#define log_str std::cout

namespace motis {
namespace reliability {
namespace distributions_calculator {

#if 0
/** method responsible for calculating a departure distribution */
bool compute_departure_distribution(
    pd_calc_data_departure const& data,
    probability_distribution& departure_distribution) {

  if (LOGGING) {
    log_str << "\n\n\n\n---------------------------------------\n\n"
            << "calculate DEPARTURE PD:\n" << std::endl;
    log_str << "dep-data:" << std::endl;
    data.debug_output(log_str);
    log_str << "\n\n" << std::endl;
  }

  // dist_only_feasible_part is a vector of modified arrival distributions
  // of the feeder trains. These distributions only contain
  // the probabilities in the waiting time interval of the departing train
  // (see random variable y in the atmos12-paper).
  std::vector<probability_distribution> dist_only_feasible_part;
  init_arr_dists_only_feasible_part(data, dist_only_feasible_part);

  // TODO: prob_feeder_late is not used
  // init_prob_feeder_late(data);

  // get the last-minute of the departure distribution
  // (this will be the highest expected delay for the departure event)
  int last_minute = data->get_latest_departure_minute();
  // first minute is always 0 since departures earlier than scheduled are not
  // allowed
  first_minute = 0;

  // index-variable for accessing the elements of the probability array
  // "probs"
  unsigned int probs_idx = 0;
  // set number of delay values for the current departure to 'last_minute + 1'.
  probs_num = last_minute + 1;

  // if the width of the distribution is smaller or equal zero,
  // return an empty distribution
  if (probs_num <= 0) {
    dep_dist->set_to_one_point_dist(0, 0.0);
    cout << "\n_warning(get_prob_dist_dep): empty "
            "distribution" << std::endl;
    return false;
  }

  if (probs_num >= 1000) {
    cout << "\n\n_error probs_num is too large!\ndep_data: " << std::endl;
    data->output(cout);
    cout << "\nfirst:" << first_minute << " last: " << last_minute
         << " num: " << probs_num << std::endl;
    cout << "arr_dist: " << data->arr_dist->get_first_minute() << " "
         << data->arr_dist->get_last_minute() << std::endl;
    assert(false);
  }

  // variable storing the probability calculated
  // for the currently processed delay minute
  probability new_prob = 0.0;
  // variable storing difference between the recalculated
  // probability and the old probability stored in the distribution
  probability diff = 0.0;
  // variable storing the greatest calculated difference
  // (see variable 'diff' and the return call at the end of this method).
  probability max_diff = 0.0;

  // iterate over all minutes of the departure distribution
  // and recalculate the corresponding probability
  for (int delay = first_minute; delay <= last_minute; delay++, probs_idx++) {

    // probs_idx has to be smaller than probs_num
    if (probs_idx >= probs_num) {
      cout << "\n_warning(get_prob_dist_dep): unexpected "
              "index" << std::endl;
      break;
    }

    // calculate the current departure time (as motis-time) depending on
    // the scheduled departure time and current delay minute
    time dep_time(data->sched_dep_time);
    dep_time.add_minutes(delay);

    /* calculate the probability of a departure at time 'dep_time' */

    // if delay is smaller than 0 set new_prob to 0 (should never happen)
    if (delay < 0) {
      new_prob = 0.0;
    }
    // for delay == 0 calculate the probability of a departure at the scheduled
    // departure time
    else if (delay == 0) {
      new_prob = get_prob_dep_at_sched_time(data);
    }
    // for a delay greater than 0 and smaller than or equal
    // maximal waiting time of the departing train for its feeders
    // calculate the probability of a departure in the waiting interval (at time
    // 'dep_time')
    else if (delay > 0 && delay <= data->wait_max) {
      new_prob = get_prob_dep_waiting_interval(data, dep_time,
                                               dist_only_feasible_part);
    }
    // for a delay greater than the maximal waiting time
    // calculate the probability of a departure after the waiting interval (at
    // time 'dep_time')
    else {
      new_prob = get_prob_dep_after_waiting_interval(data, dep_time);
    }

    // store the calculated probability in the probability array "this-probs".
    // the calculated values are stored in this array,
    // and later this array is used to initialize the
    // probability_distribution-object
    if (probs_idx >= 1000)
      cout << "\n_warning: probs_idx too large" << std::endl;
    assert(probs_idx < 1000);
    probs[probs_idx] = new_prob;

    // the maximum difference between the new and the old probabilities is
    // stored
    // in max_diff. if max_diff becomes greater than MAX_DIFF_VALUES,
    // this method will return false anyways. therefore it is not necessary
    // anymore
    // to find a greater difference.
    if (max_diff < MAX_DIFF_VALUES) {
      diff = dep_dist->get_prob_equal(delay) - probs[probs_idx];
      if (diff < 0) diff *= -1;
      if (diff > max_diff) max_diff = diff;
    }

    if (LOGGING)
      log_str << "SP[" << dep_time.output() << "]: " << probs[probs_idx]
              << std::endl
              << "\n----------------------------------" << std::endl
              << std::endl;
  }  // end of for

  // delete the array dist_only_feasible_part
  for (unsigned int i = 0; i < data->num_feeders; i++)
    delete dist_only_feasible_part[i];
  delete[] dist_only_feasible_part;

  // initialize the departure distribution
  // using the probabilities stored in "probs"
  dep_dist->init(probs, first_minute, probs_num, ignore_small_values);

  if (dep_dist->get_last_minute() < dep_dist->get_first_minute()) {
    cout << "\n_warning(get_prob_dist_dep): could not "
            "calculate departure distribution correctly" << std::endl;

    if (LOGGING) {
      log_str << "\n_warning(get_prob_dist_dep): "
                 "could not calculate departure distribution correctly"
              << std::endl;
      log_str << "first_minute: " << first_minute << " num_probs: " << probs_num
              << std::endl;
      data->output(log_str);
    }
  }

  if (LOGGING) {
    log_str << "\n_calculated departure distribution:" << std::endl;
    dep_dist->output(
        log_str, data->sched_dep_time.get_minutes_after_reference_time(), true);
  }

#ifdef CHECK_DISTS_SUM
  if (!data->has_interchange()) {
    probability sum = dep_dist->get_sum();
    if (sum < 0.99 || sum > 1.0001) {
      cout << "\n\n_data:" << std::endl;
      data->output(cout);
      cout << "\n\n_dep-dist: " << std::endl;
      dep_dist->output(
          cout, data->sched_dep_time.get_minutes_after_reference_time(), true);
      getchar();
    }
  }
#endif

  // if max_diff remains smaller than MAX_DIFF_VALUES
  // the recalculated distribution has no noticeable changes
  // compared to the old values stored in 'dep_dist'.
  // the result of this comparison is used in SGProb_dists_wrapper
  // in order to accelerate the distributions calculation process.
  return (max_diff < MAX_DIFF_VALUES);
}

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

/**
 * Calculate the probability that from the time stored in 'timestamp'
 * the departure of the train does not depend on any feeder.
 */
probability departure_independent_from_feeders(
    std::vector<pd_calc_data_departure::feeder_info> const& feeders,
    time const timestamp) {
  probability prob_no_waiting = 1.0, prob_no_waiting_feeder = 0.0,
              prob_waiting_feeder = 0.0;

  for (auto const& feeder : feeders) {
    time const waiting_interval_begin = timestamp - feeder.transfer_time_;
    if (waiting_interval_begin >= feeder.latest_feasible_arrival_) {
      prob_no_waiting_feeder = 1.0;
    } else {
      prob_waiting_feeder =
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

#if 0

/**
 * calculate the probability for a departure at the scheduled departure time.
 * @param pd_calc_data_departure* object containing all necessary data
 * @return probability probability
 */
probability get_prob_dep_at_sched_time(pd_calc_data_departure* data) {

  probability prob_res = 0.0, prob_train_early_enough = 0.0,
              prob_no_waiting_feeder = 0.0;

  // latest arrival-time of the train so that departure at scheduled time is
  // possible
  time latest_arr_time(data->sched_dep_time);
  latest_arr_time.subtract_minutes(data->min_standing);

  // probability of train being early enough so that departure at scheduled time
  // is possible
  if (data->arr_dist == NULL)
    prob_train_early_enough = 1.0;
  else
    prob_train_early_enough = data->arr_dist->get_prob_smaller_equal(
        latest_arr_time.get_difference_in_minutes(data->arr_time_train));

  // probability that there is no need to wait for a feeder
  prob_no_waiting_feeder =
      departure_independent_from_feeders(data, data->sched_dep_time);

  prob_res = prob_train_early_enough * prob_no_waiting_feeder;

  if (LOGGING)
    log_str << "get_prob_dep_at_sched_time(" << data->sched_dep_time.output()
            << "): prob_train_early_enough=" << prob_train_early_enough
            << " prob_no_wait_feeder=" << prob_no_waiting_feeder << std::endl;

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

    prob_res *= ic_feeder_early_enough;

    if (LOGGING)
      log_str << "ic_feeder_early_enough=" << ic_feeder_early_enough
              << std::endl;
  }

  return prob_res;
}

/**
 * calculate the probability that the departing train has to wait for
 * at least one feeder at a point in time stored in parameter 'timestamp'.
 *
 * @param pd_calc_data_departure* object containing all necessary data
 * @param const time& point in time
 * @param probability_distribution** arr_dists_only_feasible_part corresponding
 *modified arrival distributions of the feeders
 *                           (see comments of method init_arr_dists_y)
 * @return probability probability
 */
probability get_prob_waiting_for_feeders(
    pd_calc_data_departure* data, const time& timestamp,
    probability_distribution** arr_dists_only_feasible_part) {

  if (LOGGING)
    log_str << "get_prob_waiting_for_feeders(" << format_time(timestamp)
            << "): " << std::endl;

  if (data->num_feeders <= 0) return 0.0;

  probability prob_max_feeders_delay1 = 1.0, prob_max_feeders_delay2 = 1.0;
  int feeder_delay = 0;
  time feeder_arrival_time;

  for (unsigned int i = 0; i < data->num_feeders; i++) {

    // if there is an interchange before the departure event
    // ignore the feeder involved in the interchange
    // (this feeder will be considered separately)
    if (data->has_interchange() && data->get_i_c_feeder_idx() == i) continue;

    // arrival-time of the feeder
    // note: in the formula presented in the paper "stochastic analysis of
    // delays in train connections"
    // we added the transfer-time to the random variable y. here, we substract
    // the transfer-time
    // from "time". this has the same effect.
    feeder_arrival_time = timestamp;
    feeder_arrival_time.subtract_minutes(data->transfer_times[i]);

    // arrival-delay of the feeder
    feeder_delay = feeder_arrival_time.get_difference_in_minutes(
        data->arr_time_feeders[i]);

#if 0
    // multiply the probabilities of the feeder to have a maximal delay of 'feeder_delay'.
    // we also have to count the probability that the feeder arrives after its latest feasible arrival time
    // since we are looking for the latest arrival time within the waiting-interval and
    // sum up the probabilities that the feeder arrives early enough or too late
    // so that its arrival time does not exceed the maximum.

    prob_max_feeders_delay1 *= (arr_dists_only_feasible_part[i]->get_prob_smaller_equal(feeder_delay) + prob_feeder_late[i]);
    // multiply the probabilities of the feeder to have a maximal delay of 'feeder_delay - 1'
    prob_max_feeders_delay2 *= (arr_dists_only_feasible_part[i]->get_prob_smaller_equal(feeder_delay - 1) + prob_feeder_late[i]);
#else

    // here we calculate the probability that the feeder is not delayed more
    // than "feeder_delay".
    // note that probabilities after latest-feasible-arrival time are set to 0.

    prob_max_feeders_delay1 *=
        (1.0 - arr_dists_only_feasible_part[i]->get_prob_greater(feeder_delay));

    prob_max_feeders_delay2 *=
        (1.0 -
         arr_dists_only_feasible_part[i]->get_prob_greater(feeder_delay - 1));

#endif

    if (LOGGING) {
      log_str << "feeder[" << i << "]: (arr_time: "
              << time(data->arr_time_feeders[i], feeder_delay).output() << ") "
              << arr_dists_only_feasible_part[i]->get_prob_smaller_equal(
                     feeder_delay) << " "
              << arr_dists_only_feasible_part[i]->get_prob_smaller_equal(
                     feeder_delay - 1) << std::endl;
    }
  }

  probability prob_max_feeders_delay =
      prob_max_feeders_delay1 - prob_max_feeders_delay2;

  if (LOGGING)
    log_str << "prob-waiting: " << prob_max_feeders_delay1 << " - "
            << prob_max_feeders_delay2 << " = "
            << (prob_max_feeders_delay1 - prob_max_feeders_delay2) << std::endl;

  return prob_max_feeders_delay;
}

/**
 * calculate the probability for a departure in the waiting interval
 * at a point in time stored in parameter 'timestamp'. this method is called
 * when there is no interchange before this departure event.
 *
 * @param pd_calc_data_departure* object containing all necessary data
 * @param const time& point in time
 * @param probability_distribution** arr_dists_only_feasible_part corresponding
 *modified arrival distributions of the feeders
 *                           (see comments of method init_arr_dists_y)
 * @return probability probability
 */
probability get_prob_dep_waiting_interval_no_interchange(
    pd_calc_data_departure* data, const time& timestamp,
    probability_distribution** arr_dists_only_feasible_part) {

  probability prob_res = 0.0, prob_train_has_delay = 0.0,
              prob_a_feeder_has_delay = 0.0;

  // latest arrival-time of the train so that a departure at the given time is
  // possible
  time latest_arr_time(timestamp);
  latest_arr_time.subtract_minutes(data->min_standing);

  // probability that the train arrives at the given time
  probability prob_arr_at_time = 0.0;
  // probability that the train arrives before the given time
  probability prob_arr_before_time = 1.0;
  if (data->arr_dist != NULL) {
    int delay = latest_arr_time.get_difference_in_minutes(data->arr_time_train);
    prob_arr_at_time = data->arr_dist->get_prob_equal(delay);
    prob_arr_before_time = data->arr_dist->get_prob_smaller(delay);
  }

  // probability that there is no need to wait for a feeder train
  probability prob_no_waiting_for_feeders =
      departure_independent_from_feeders(data, timestamp);

  // probability that the departure at the given time is because of the delay of
  // the train itself
  prob_train_has_delay = prob_arr_at_time * prob_no_waiting_for_feeders;
  // probability that the departure at the given time is because of waiting of
  // at least one feeder
  prob_a_feeder_has_delay =
      prob_arr_before_time * get_prob_waiting_for_feeders(
                                 data, timestamp, arr_dists_only_feasible_part);

  if (LOGGING)
    log_str << "get_prob_dist_dep_waiting_interval(" << format_time(timestamp)
            << "): " << std::endl
            << "prob_arr_at_time(" << latest_arr_time.output()
            << "):" << prob_arr_at_time
            << " * prob_no_waiting = " << prob_train_has_delay << std::endl
            << " + prob_arr_before_time(" << latest_arr_time.output()
            << "):" << prob_arr_before_time
            << " * prob_waiting = " << prob_a_feeder_has_delay << std::endl;

  prob_res = (prob_train_has_delay + prob_a_feeder_has_delay);

  return prob_res;
}

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

/**
 * calculate the probability for a departure in the waiting interval
 * at a point in time stored in parameter 'timestamp'.
 * this method calls get_prob_dep_waiting_interval_no_interchange or
 * get_prob_dep_waiting_interval_interchange depending on whether
 * there is an interchange before this departure event.
 *
 * at time stored in parameter 'time'.
 * @param pd_calc_data_departure* object containing all necessary data
 * @param const time& point in time
 * @param probability_distribution** arr_dists_only_feasible_part corresponding
 *modified arrival distributions of the feeders
 *                           (see comments of method init_arr_dists_y)
 * @return probability probability
 */
probability get_prob_dep_waiting_interval(
    pd_calc_data_departure* data, const time& timestamp,
    probability_distribution** arr_dists_only_feasible_part) {

  probability prob = 0.0;

  if (data->has_interchange())
    prob = get_prob_dep_waiting_interval_interchange(
        data, timestamp, arr_dists_only_feasible_part);
  else
    prob = get_prob_dep_waiting_interval_no_interchange(
        data, timestamp, arr_dists_only_feasible_part);

  return prob;
}

/**
 * calculate the probability for a departure after the waiting interval
 * at a point in time stored in parameter 'timestamp'.
 * after the waiting time interval the train does not have to wait for any
 *feeder.
 * this method calls get_prob_dep_waiting_interval_no_interchange or
 * get_prob_dep_waiting_interval_interchange depending on whether
 * there is an interchange before this departure event.
 *
 * @param pd_calc_data_departure* object containing all necessary data
 * @param const time& point in time
 * @param probability_distribution** arr_dists_only_feasible_part corresponding
 *modified arrival distributions of the feeders
 *                           (see comments of method init_arr_dists_y)
 * @return probability probability
 */
probability get_prob_dep_after_waiting_interval(pd_calc_data_departure* data,
                                                const time& timestamp) {

  if (data->arr_dist == NULL) return 0.0;

  // latest arrival-time of the train so that a departure at the given time is
  // possible
  time latest_arr_time(timestamp);
  latest_arr_time.subtract_minutes(data->min_standing);
  int train_delay =
      latest_arr_time.get_difference_in_minutes(data->arr_time_train);

  // get the probability that the train arrives with a delay of 'train_delay'
  probability prob_res = data->arr_dist->get_prob_equal(train_delay);

  if (LOGGING)
    log_str << "get_prob_dist_dep_after_waiting_interval("
            << format_time(timestamp) << "): " << prob_res << std::endl;

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
    prob_res *= prob_no_waiting_for_i_c_feeder;

    if (LOGGING)
      log_str << "prob_no_wait_for_i_c_feeder: "
              << prob_no_waiting_for_i_c_feeder << std::endl;
  }

  return prob_res;
}

/**
 * Feeder distributions are modified to contain only probabilities
 * which have an influence on the departure distribution.
 */
void get_feasible_part_of_distributions(
    pd_calc_data_departure const& data,
    std::vector<probability_distribution>& feeder_distributions_feasible_part) {

  feeder_distributions_feasible_part(data.feeders_.size());

  for (unsigned int feeder_idx = 0; feeder_idx < data.feeders_.size();
       feeder_idx++) {
    auto const& feeder = data.feeders_[feeder_idx];
    std::vector<probability> probabilities;
    int first_minute = feeder.distribution_.first_minute();

    time lfa_minute = feeder.latest_feasible_arrival_ - feeder.arrival_time_;

    for (int feeder_delay = feeder.distribution_.first_minute();
         feeder_delay <= feeder.distribution_.last_minute(); feeder_delay++) {
      if (feeder_delay > lfa_minute)
        probabilities.push_back(0.0);
      else
        probabilities.push_back(
            feeder.distribution_.probability_equal(feeder_delay));
    }

    feeder_distributions_feasible_part[feeder_idx].init(probabilities,
                                                        first_minute);
  }
}

#endif

}  // namespace detail
}  // namespace distributions_calculator
}  // namespace reliability
}  // namespace motis
