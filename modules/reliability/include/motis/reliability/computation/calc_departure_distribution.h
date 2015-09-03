#pragma once

#include <vector>

#include "data_departure.h"
#include "motis/core/schedule/time.h"

#include "motis/reliability/probability_distribution.h"

namespace motis {
namespace reliability {
namespace calc_departure_distribution {

void compute_departure_distribution(
    data_departure const& data,
    probability_distribution& departure_distribution);

namespace detail {

/**
 * calculate the probability for a departure at the scheduled departure time.
 */
probability departure_at_scheduled_time(data_departure const& data);

/**
 * Calculate the probability for a departure in the waiting interval
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
    time const timestamp);

/**
 * Calculate the probability for a departure after the waiting interval
 * at the point in time stored in 'timestamp'.
 */
probability departure_after_waiting_interval(data_departure const& data,
                                             time const timestamp);

/**
 * Calculate the probability that from the time stored in 'timestamp'
 * the departure of the train does not depend on any feeder.
 */
probability departure_independent_from_feeders(
    std::vector<data_departure::feeder_info> const& feeders,
    time const timestamp);

/**
 * Calculate the probability that the train has to wait until the time stored in
 * 'timestamp' for at least one feeder and there is no other feeder
 * for which the departing train would wait for a longer time.
 * In other words, this is the probability that the train departs at the time
 * stored in 'timestamp' since it has waited for at least one feeder until this
 * time.
 *
 * @param modified_feeders_distributions: vector containing for each feeder
 *        those minutes of the arrival distribution which are not later than the
 *        latest feasible arrival time of the feeder.
 *        This modified distributions are determined by
 *        'cut_minutes_after_latest_feasible_arrival'.
 */
probability had_to_wait_for_feeders(
    std::vector<data_departure::feeder_info> const& feeders,
    std::vector<probability_distribution> const& modified_feeders_distributions,
    time const timestamp);

/**
 * Cut all minutes which are later than the
 * latest feasible arrival time of the feeder.
 */
void cut_minutes_after_latest_feasible_arrival(
    std::vector<data_departure::feeder_info> const& feeders,
    std::vector<probability_distribution>& part_before_lfa);

/* helper for departure_at_scheduled_time */
probability train_early_enough(data_departure const& data);

/* helper for departure_within_waiting_interval and
 * departure_after_waiting_interval */
probability train_arrives_before_time(data_departure const& data,
                                      time const timestamp);

/* helper for departure_within_waiting_interval and
 * departure_after_waiting_interval */
probability train_arrives_at_time(data_departure const& data,
                                  time const timestamp);
}
}

}  // namespace reliability
}  // namespace motis
