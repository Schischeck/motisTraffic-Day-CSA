#include "catch/catch.hpp"

#include <iostream>

#include "motis/core/common/date_util.h"

#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

#include "motis/loader/loader.h"

#include "motis/reliability/distributions_container.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/probability_distribution.h"
#include "motis/reliability/computation/calc_departure_distribution.h"
#include "motis/reliability/computation/data_departure.h"

#include "include/precomputed_distributions_test_container.h"
#include "include/start_and_travel_test_distributions.h"

using namespace motis;
using namespace motis::reliability;
using namespace motis::reliability::calc_departure_distribution;
using namespace motis::reliability::calc_departure_distribution::detail;

namespace schedule1 {
/* train numbers */
short const IC_DA_H = 1;
short const IC_FR_DA = 2;
short const IC_FH_DA = 3;
short const RE_MA_DA = 4;
short const ICE_FR_DA_H = 5;
short const ICE_HA_W_HE = 6;
short const ICE_K_K = 7;
short const RE_K_S = 8;
}

TEST_CASE("departure_independent_from_feeders",
          "[calc_departure_distribution]") {
  std::vector<data_departure::feeder_info> feeders;
  probability_distribution feeder1_dist;
  feeder1_dist.init({0.4, 0.3, 0.2, 0.1}, 0);  // distribution from 13 to 16
  feeders.emplace_back(feeder1_dist, 13, 15, 5);  // sched, lfa, transfers

  // independent since departure - transfer is later than lfa
  REQUIRE(equal(departure_independent_from_feeders(feeders, 21), 1.0));
  // independent since departure - transfer equals lfa
  REQUIRE(equal(departure_independent_from_feeders(feeders, 20), 1.0));
  // indepenetend from the first feeder if it arrives at 13, 14, or 16
  REQUIRE(equal(departure_independent_from_feeders(feeders, 19), 0.8));
  // independent if train arrives at 13 or 16
  REQUIRE(equal(departure_independent_from_feeders(feeders, 18), 0.5));
  // independent if train arrives at 16
  REQUIRE(equal(departure_independent_from_feeders(feeders, 17), 0.1));
  // independent if train arrives at 16
  REQUIRE(equal(departure_independent_from_feeders(feeders, 16), 0.1));

  probability_distribution feeder2_dist;
  feeder2_dist.init({0.1, 0.5, 0.3, 0.1}, -1);  // distribution from 13 to 16
  feeders.emplace_back(feeder2_dist, 14, 16, 5);  // sched, lfa, transfers

  // independent since departure - transfer is later than lfa of both feeders
  REQUIRE(equal(departure_independent_from_feeders(feeders, 22), 1.0));
  // independent since departure - transfer is later than feeder1-lfa and equals
  // feeder2-lfa
  REQUIRE(equal(departure_independent_from_feeders(feeders, 21), 1.0));
  // independent from the first feeder since departure - transfer equals lfa
  // and independent from the second feeder if it arrives at 13-15
  REQUIRE(equal(departure_independent_from_feeders(feeders, 20), 1.0 * 0.9));
  // independent from the first feeder if it arrives at 13, 14, or 16
  // and independent from the second feeder if it arrives at 13 or 14
  REQUIRE(equal(departure_independent_from_feeders(feeders, 19), 0.8 * 0.6));
  // independent from the first feeder if it arrives at 13 or 16
  // and independent from the second feeder if it arrives at 13
  REQUIRE(equal(departure_independent_from_feeders(feeders, 18), 0.5 * 0.1));
  // independent from the first feeder if it arrives at 16
  // and depends on the second feeder
  REQUIRE(equal(departure_independent_from_feeders(feeders, 17), 0.1 * 0.0));
  // independent from the first feeder if it arrives at 16
  // and depends on the second feeder
  REQUIRE(equal(departure_independent_from_feeders(feeders, 16), 0.1 * 0.0));
}

// first route node
TEST_CASE("train_early_enough1", "[calc_departure_distribution]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));

  distributions_container::precomputed_distributions_container dummy(0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule, schedule1::ICE_FR_DA_H);
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& first_light_conn = first_route_edge->_m._route_edge._conns[0];

  data_departure data(first_route_node, first_light_conn, true, *schedule,
                      dummy, dummy, s_t_distributions);
  train_arrived(data);

  REQUIRE(equal(train_arrived(data), 0.6));
}

// preceding arrival
TEST_CASE("train_early_enough2", "[calc_departure_distribution]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));

  precomputed_distributions_test_container train_distributions({0.1, 0.7, 0.2},
                                                               -1);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route node at Hanau of train ICE_HA_W_HE
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule, schedule1::ICE_HA_W_HE);
  // route node at Wuerzburg
  auto second_route_node =
      graph_accessor::get_departing_route_edge(first_route_node)->_to;
  // route edge from Wuerzburg to Heilbronn
  auto const route_edge =
      graph_accessor::get_departing_route_edge(*second_route_node);
  auto const& light_connection = route_edge->_m._route_edge._conns[0];

  data_departure data(*second_route_node, light_connection, false, *schedule,
                      train_distributions, train_distributions,
                      s_t_distributions);
  train_arrived(data);

  REQUIRE(equal(train_arrived(data), 0.8));
}

TEST_CASE("cut_minutes_after_latest_feasible_arrival1",
          "[calc_departure_distribution]") {
  probability_distribution feeder1_dist;
  std::vector<data_departure::feeder_info> feeders;
  std::vector<probability_distribution> modified_distributions;

  feeder1_dist.init({0.4, 0.3, 0.2, 0.1}, 0);  // distribution from 13 to 16
  feeders.emplace_back(feeder1_dist, 13, 15, 5);  // sched, lfa, transfers

  cut_minutes_after_latest_feasible_arrival(feeders, modified_distributions);

  REQUIRE(modified_distributions.size() == 1);
  REQUIRE(modified_distributions[0].first_minute() == 0);
  REQUIRE(equal(modified_distributions[0].probability_smaller_equal(-1), 0.0));
  REQUIRE(equal(modified_distributions[0].probability_equal(0), 0.4));
  REQUIRE(equal(modified_distributions[0].probability_equal(1), 0.3));
  REQUIRE(equal(modified_distributions[0].probability_equal(2), 0.2));
  REQUIRE(equal(modified_distributions[0].probability_greater_equal(3), 0.0));
}

TEST_CASE("cut_minutes_after_latest_feasible_arrival2",
          "[calc_departure_distribution]") {
  probability_distribution feeder1_dist;
  std::vector<data_departure::feeder_info> feeders;
  std::vector<probability_distribution> modified_distributions;

  feeder1_dist.init({0.1, 0.5, 0.3, 0.1}, -1);  // distribution from 10 to 13
  feeders.emplace_back(feeder1_dist, 11, 14, 5);  // sched, lfa, transfers

  probability_distribution feeder2_dist;
  feeder2_dist.init({0.1, 0.5, 0.3, 0.1}, 0);  // distribution from 15 to 18
  feeders.emplace_back(feeder2_dist, 15, 15, 5);  // sched, lfa, transfers

  cut_minutes_after_latest_feasible_arrival(feeders, modified_distributions);

  REQUIRE(modified_distributions.size() == 2);

  REQUIRE(modified_distributions[0].first_minute() == -1);
  REQUIRE(equal(modified_distributions[0].probability_smaller_equal(-2), 0.0));
  REQUIRE(equal(modified_distributions[0].probability_equal(-1), 0.1));
  REQUIRE(equal(modified_distributions[0].probability_equal(0), 0.5));
  REQUIRE(equal(modified_distributions[0].probability_equal(1), 0.3));
  REQUIRE(equal(modified_distributions[0].probability_equal(2), 0.1));
  REQUIRE(equal(modified_distributions[0].probability_greater_equal(3), 0.0));

  REQUIRE(modified_distributions[1].first_minute() == 0);
  REQUIRE(equal(modified_distributions[1].probability_smaller_equal(-1), 0.0));
  REQUIRE(equal(modified_distributions[1].probability_equal(0), 0.1));
  REQUIRE(equal(modified_distributions[1].probability_greater_equal(1), 0.0));
}

TEST_CASE("had_to_wait_for_feeders1", "[departure_data]") {
  probability_distribution feeder1_dist;
  std::vector<data_departure::feeder_info> feeders;
  std::vector<probability_distribution> modified_distributions;

  feeder1_dist.init({0.1, 0.5, 0.3, 0.1}, -1);  // distribution from 10 to 13
  feeders.emplace_back(feeder1_dist, 11, 12, 5);  // sched, lfa, transfers

  probability_distribution feeder2_dist;
  feeder2_dist.init({0.1, 0.5, 0.3, 0.1}, 0);  // distribution from 11 to 14
  feeders.emplace_back(feeder2_dist, 11, 13, 5);  // sched, lfa, transfers

  /* get modified feeder distributions:
   * feeder 1: 10=0.1, 11=0.5, 12=0.3, 13=0.0, 14=0.0
   * feeder 2: 10=0.0, 11=0.1, 12=0.5, 13=0.3, 14=0.0 */
  cut_minutes_after_latest_feasible_arrival(feeders, modified_distributions);

  // Time 16 - 5 = 11
  // probability of waiting for feeder 1 and not for feeder 2:
  // 0.5 * (1 - (0.1 + 0.5 + 0.3)) = 0.05
  // probability of waiting for feeder 2 and not for feeder 1:
  // 0.1 * (1 - (0.5 + 0.3)) = 0.02
  // probability of waiting for both: 0.5 * 0.1 = 0.05
  REQUIRE(equal(had_to_wait_for_feeders(feeders, modified_distributions, 16),
                0.12));

  // Time 17 - 5 = 12
  // probability of waiting for feeder 1 and not for feeder 2:
  // 0.3 * (1 - (0.5 + 0.3)) = 0.06
  // probability of waiting for feeder 2 and not for feeder 1:
  // 0.5 * (1 - 0.3) = 0.35
  // probability of waiting for both: 0.3 * 0.5 = 0.15
  REQUIRE(equal(had_to_wait_for_feeders(feeders, modified_distributions, 17),
                0.56));

  // Time 18 - 5 = 13
  // probability of waiting for feeder 1 and not for feeder 2: 0.0
  // probability of waiting for feeder 2 and not for feeder 1: 0.3
  // probability of waiting for both feeders: 0.0
  REQUIRE(
      equal(had_to_wait_for_feeders(feeders, modified_distributions, 18), 0.3));

  // Time 19 - 5 = 14
  REQUIRE(
      equal(had_to_wait_for_feeders(feeders, modified_distributions, 19), 0.0));
}

/* different transfer times */
TEST_CASE("had_to_wait_for_feeders2", "[departure_data]") {
  probability_distribution feeder1_dist;
  std::vector<data_departure::feeder_info> feeders;
  std::vector<probability_distribution> modified_distributions;

  feeder1_dist.init({0.1, 0.5, 0.3, 0.1}, -1);  // distribution from 10 to 13
  feeders.emplace_back(feeder1_dist, 11, 12, 5);  // sched, lfa, transfers

  probability_distribution feeder2_dist;
  feeder2_dist.init({0.1, 0.5, 0.3, 0.1}, 0);  // distribution from 11 to 14
  feeders.emplace_back(feeder2_dist, 11, 13, 4);  // sched, lfa, transfers

  /* get modified feeder distributions:
   * feeder 1: 10=0.1, 11=0.5, 12=0.3, 13=0.0, 14=0.0
   * feeder 2: 10=0.0, 11=0.1, 12=0.5, 13=0.3, 14=0.0 */
  cut_minutes_after_latest_feasible_arrival(feeders, modified_distributions);

  // Time feeder 1: 16 - 5 = 11
  // Time feeder 2: 16 - 4 = 12
  // probability of waiting for feeder 1 and not for feeder 2:
  // 0.5 * (1 - (0.5 + 0.3)) = 0.1
  // probability of waiting for feeder 2 and not for feeder 1:
  // 0.5 * (1 - (0.5 + 0.3)) = 0.1
  // probability of waiting for both: 0.5 * 0.5 = 0.25
  REQUIRE(equal(had_to_wait_for_feeders(feeders, modified_distributions, 16),
                0.45));

  // Time feeder 1: 17 - 5 = 12
  // Time feeder 2: 17 - 4 = 13
  // probability of waiting for feeder 1 and not for feeder 2:
  // 0.3 * (1 - 0.3) = 0.21
  // probability of waiting for feeder 2 and not for feeder 1:
  // 0.3 * (1 - 0.3) = 0.21
  // probability of waiting for both: 0.3 * 0.3 = 0.09
  REQUIRE(equal(had_to_wait_for_feeders(feeders, modified_distributions, 17),
                0.51));

  // Time feeder 1: 18 - 5 = 13
  // Time feeder 2: 18 - 4 = 14
  // probability of waiting for feeder 1 and not for feeder 2: 0.0
  // probability of waiting for feeder 2 and not for feeder 1: 0.0
  // probability of waiting for both feeders: 0.0
  REQUIRE(
      equal(had_to_wait_for_feeders(feeders, modified_distributions, 18), 0.0));

  // Time feeder 1: 19 - 5 = 14
  // Time feeder 2: 19 - 4 = 15
  REQUIRE(
      equal(had_to_wait_for_feeders(feeders, modified_distributions, 19), 0.0));
}

// first route node without feeders
TEST_CASE("compute_departure_distribution1", "[calc_departure_distribution]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));

  distributions_container::precomputed_distributions_container dummy(0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule, schedule1::ICE_FR_DA_H);
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& first_light_conn = first_route_edge->_m._route_edge._conns[0];

  data_departure data(first_route_node, first_light_conn, true, *schedule,
                      dummy, dummy, s_t_distributions);
  probability_distribution departure_distribution;
  compute_departure_distribution(data, departure_distribution);

  REQUIRE(departure_distribution.first_minute() == 0);
  REQUIRE(departure_distribution.last_minute() == 1);
  REQUIRE(equal(departure_distribution.sum(), 1.0));
  REQUIRE(equal(departure_distribution.probability_equal(0), 0.6));
  REQUIRE(equal(departure_distribution.probability_equal(1), 0.4));
}

// first route node with feeders
TEST_CASE("compute_departure_distribution2", "[calc_departure_distribution]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));

  precomputed_distributions_test_container train_distributions(
      {0.1, 0.4, 0.1, 0.1, 0.1, 0.1, 0.1}, -1);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route node at Darmstadt of train IC_DA_H
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule, schedule1::IC_DA_H);
  // route edge from Darmstadt to Heidelberg
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  // light connection d07:00 a07:28
  auto const& light_connection = first_route_edge->_m._route_edge._conns[1];

  /* Scheduled departure time: 07:00
   * First Feeder:
   *   feeder-arrival-time: 06:54, lfa: 06:58, transfer-time: 5
   *   06:53=0.1, 06:54=0.4, 06:55=0.1, 06:56=0.1,
   *   06:57=0.1, 06:58=0.1, 06:59=0.1
   * The second feeder has no influence on this departure
   */
  data_departure data(first_route_node, light_connection, true, *schedule,
                      train_distributions, train_distributions,
                      s_t_distributions);

  REQUIRE(
      equal(train_arrives_at_time(data, data.scheduled_departure_time_), 0.6));
  REQUIRE(equal(train_arrives_at_time(data, data.scheduled_departure_time_ + 1),
                0.4));
  REQUIRE(equal(train_arrives_at_time(data, data.scheduled_departure_time_ + 2),
                0.0));
  REQUIRE(
      equal(train_arrives_before_time(data, data.scheduled_departure_time_ + 1),
            0.6));
  REQUIRE(
      equal(train_arrives_before_time(data, data.scheduled_departure_time_ + 2),
            1.0));

  REQUIRE(equal(departure_independent_from_feeders(
                    data.feeders_, data.scheduled_departure_time_ + 1),
                0.8));

  probability_distribution departure_distribution;
  compute_departure_distribution(data, departure_distribution);

  /* [sum=1 first-min=0 last-min=3 values=0.42,0.38,0.1,0.1] */

  REQUIRE(departure_distribution.first_minute() == 0);
  REQUIRE(departure_distribution.last_minute() == 3);
  REQUIRE(equal(departure_distribution.sum(), 1.0));
  REQUIRE(equal(departure_distribution.probability_equal(0),
                0.6 * (0.1 + 0.4 + 0.1 + 0.1)));
  REQUIRE(equal(departure_distribution.probability_equal(1),
                0.4 * (0.1 + 0.4 + 0.1 + 0.1 + 0.1) + 0.6 * 0.1));
  REQUIRE(equal(departure_distribution.probability_equal(2), 0.1));
  REQUIRE(equal(departure_distribution.probability_equal(3), 0.1));
}

// route node with preceding arrival and without feeders
TEST_CASE("compute_departure_distribution3", "[calc_departure_distribution]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));

  precomputed_distributions_test_container train_distributions({0.1, 0.7, 0.2},
                                                               -1);
  start_and_travel_test_distributions const* dummy = nullptr;

  // route node at Hanau of train ICE_HA_W_HE
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule, schedule1::ICE_HA_W_HE);
  // route node at Wuerzburg
  auto second_route_node =
      graph_accessor::get_departing_route_edge(first_route_node)->_to;
  // route edge from Wuerzburg to Heilbronn
  auto const route_edge =
      graph_accessor::get_departing_route_edge(*second_route_node);
  auto const& light_connection = route_edge->_m._route_edge._conns[0];

  /* scheduled-departure-time: 10:34 largest-delay: 1 is-first-route-node: 0
   * preceding-arrival-time: 10:32 min-standing: 2
   * preceding-arrival-distribution: 10:31=0.1, 10:32=0.7, 10:33=0.2
   * no feeders. */
  data_departure data(*second_route_node, light_connection, false, *schedule,
                      train_distributions, train_distributions, *dummy);

  REQUIRE(equal(train_arrived(data), 0.8));
  REQUIRE(
      equal(train_arrives_at_time(data, data.scheduled_departure_time_), 0.7));
  REQUIRE(equal(train_arrives_at_time(data, data.scheduled_departure_time_ + 1),
                0.2));
  REQUIRE(equal(train_arrives_at_time(data, data.scheduled_departure_time_ + 2),
                0.0));
  REQUIRE(equal(train_arrives_before_time(data, data.scheduled_departure_time_),
                0.1));
  REQUIRE(
      equal(train_arrives_before_time(data, data.scheduled_departure_time_ + 1),
            0.8));
  REQUIRE(
      equal(train_arrives_before_time(data, data.scheduled_departure_time_ + 2),
            1.0));

  probability_distribution departure_distribution;
  compute_departure_distribution(data, departure_distribution);

  /* [sum=1 first-min=0 last-min=1 values=0.8,0.2] */

  REQUIRE(departure_distribution.first_minute() == 0);
  REQUIRE(departure_distribution.last_minute() == 1);
  REQUIRE(equal(departure_distribution.sum(), 1.0));
  REQUIRE(equal(departure_distribution.probability_equal(0), 0.8));
  REQUIRE(equal(departure_distribution.probability_equal(1), 0.2));
}

// route node with preceding arrival and feeders
TEST_CASE("compute_departure_distribution4", "[calc_departure_distribution]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));

  std::vector<probability> values;
  values.push_back(0.043);
  for (unsigned int i = 0; i < 29; i++) {
    values.push_back(0.033);
  }

  precomputed_distributions_test_container train_distributions(values, 0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route node at Darmstadt of train ICE_FR_DA_H
  auto& route_node = *graph_accessor::get_departing_route_edge(
                          *graph_accessor::get_first_route_node(
                              *schedule, schedule1::ICE_FR_DA_H))->_to;
  auto const& light_connection = graph_accessor::get_departing_route_edge(
                                     route_node)->_m._route_edge._conns[0];

  /* scheduled-departure-time: 06:11 largest-delay: 3
   * preceding-arrival-time: 06:05 min-standing: 2
   * preceding-arrival-distribution: 06:05=0.043, 06:06=0.033, ..., 06:34=0.033
   * Feeders:
   * feeder-arrival-time: 05:41 lfa: 06:09 transfer-time: 5
   * feeder-distribution: 05:41=0.043, 05:42=0.033, ..., 06:10=0.033
   *
   * feeder-arrival-time: 05:56 lfa: 06:09 transfer-time: 5
   * feeder-distribution: 05:56=0.043, 05:57=0.033, ..., 06:25=0.033
   */
  data_departure data(route_node, light_connection, false, *schedule,
                      train_distributions, train_distributions,
                      s_t_distributions);

  std::vector<probability_distribution> modified_feeders_distributions;
  detail::cut_minutes_after_latest_feasible_arrival(
      data.feeders_, modified_feeders_distributions);

  /* dep-dist: 0=0.142065, 1=0.0393849, 2=0.0439064, 3=0.0486436,
     * 4=0.033, ..., 25=0.033 */
  probability_distribution departure_distribution;
  compute_departure_distribution(data, departure_distribution);

  REQUIRE(departure_distribution.first_minute() == 0);
  REQUIRE(departure_distribution.last_minute() == 25);
  REQUIRE(equal(departure_distribution.sum(), 1.0));

  /******************* minute 0 *******************/

  REQUIRE(equal(train_arrived(data), 0.043 + 4 * 0.033));
  REQUIRE(equal(departure_independent_from_feeders(
                    data.feeders_, data.scheduled_departure_time_),
                (1.0 - 3 * (0.033)) * (1.0 - 3 * (0.033))));

  // probability 0=0.142065
  REQUIRE(equal(departure_distribution.probability_equal(0), 0.142065));
  REQUIRE(equal(departure_distribution.probability_equal(0),
                train_arrived(data) *
                    departure_independent_from_feeders(
                        data.feeders_, data.scheduled_departure_time_)));

  /******************* minute 1 *******************/

  REQUIRE(equal(departure_independent_from_feeders(
                    data.feeders_, data.scheduled_departure_time_ + 1),
                (1.0 - 2 * (0.033)) * (1.0 - 2 * (0.033))));
  REQUIRE(equal(train_arrives_at_time(data, data.scheduled_departure_time_ + 1),
                0.033));
  REQUIRE(
      equal(train_arrives_before_time(data, data.scheduled_departure_time_ + 1),
            0.043 + 4 * 0.033));
  {
    // feeder1 = 06:07 * (feeder2 < 06:07 + feeder2 > 06:09)
    probability const prob_feeder1 =
        data.feeders_[0].distribution_.probability_equal(
            timestamp_to_delay(data.feeders_[0].arrival_time_, 6 * 60 + 7)) *
        (data.feeders_[1].distribution_.probability_smaller(
             timestamp_to_delay(data.feeders_[1].arrival_time_, 6 * 60 + 7)) +
         data.feeders_[1].distribution_.probability_greater(
             timestamp_to_delay(data.feeders_[1].arrival_time_, 6 * 60 + 9)));
    // feeder2 = 06:07 * (feeder1 < 06:07 + feeder1 > 06:09)
    probability const prob_feeder2 =
        data.feeders_[1].distribution_.probability_equal(
            timestamp_to_delay(data.feeders_[1].arrival_time_, 6 * 60 + 7)) *
        (data.feeders_[0].distribution_.probability_smaller(
             timestamp_to_delay(data.feeders_[0].arrival_time_, 6 * 60 + 7)) +
         data.feeders_[0].distribution_.probability_greater(
             timestamp_to_delay(data.feeders_[0].arrival_time_, 6 * 60 + 9)));
    // feeder1 = 06:07 * feeder2 = 06:07
    probability const prob_both_feeders =
        data.feeders_[0].distribution_.probability_equal(
            timestamp_to_delay(data.feeders_[0].arrival_time_, 6 * 60 + 7)) *
        data.feeders_[1].distribution_.probability_equal(
            timestamp_to_delay(data.feeders_[1].arrival_time_, 6 * 60 + 7));

    REQUIRE(equal(
        had_to_wait_for_feeders(data.feeders_, modified_feeders_distributions,
                                data.scheduled_departure_time_ + 1),
        prob_feeder1 + prob_feeder2 + prob_both_feeders));
  }

  // probability 1=0.0393849
  REQUIRE(equal(departure_distribution.probability_equal(1), 0.0393849));
  REQUIRE(equal(
      departure_distribution.probability_equal(1),
      (departure_independent_from_feeders(data.feeders_,
                                          data.scheduled_departure_time_ + 1) *
           train_arrives_at_time(data, data.scheduled_departure_time_ + 1) +
       (train_arrives_before_time(data, data.scheduled_departure_time_ + 1) *
        had_to_wait_for_feeders(data.feeders_, modified_feeders_distributions,
                                data.scheduled_departure_time_ + 1)))));

  /******************* minute 2 *******************/

  REQUIRE(equal(departure_independent_from_feeders(
                    data.feeders_, data.scheduled_departure_time_ + 2),
                (1.0 - 0.033) * (1.0 - 0.033)));
  REQUIRE(equal(train_arrives_at_time(data, data.scheduled_departure_time_ + 2),
                0.033));
  REQUIRE(
      equal(train_arrives_before_time(data, data.scheduled_departure_time_ + 2),
            0.043 + 5 * 0.033));

  {
    // feeder1 = 06:08 * (feeder2 < 06:08 + feeder2 > 06:09)
    probability const prob_feeder1 =
        data.feeders_[0].distribution_.probability_equal(
            timestamp_to_delay(data.feeders_[0].arrival_time_, 6 * 60 + 8)) *
        (data.feeders_[1].distribution_.probability_smaller(
             timestamp_to_delay(data.feeders_[1].arrival_time_, 6 * 60 + 8)) +
         data.feeders_[1].distribution_.probability_greater(
             timestamp_to_delay(data.feeders_[1].arrival_time_, 6 * 60 + 9)));
    // feeder2 = 06:08 * (feeder1 < 06:08 + feeder1 > 06:09)
    probability const prob_feeder2 =
        data.feeders_[1].distribution_.probability_equal(
            timestamp_to_delay(data.feeders_[1].arrival_time_, 6 * 60 + 8)) *
        (data.feeders_[0].distribution_.probability_smaller(
             timestamp_to_delay(data.feeders_[0].arrival_time_, 6 * 60 + 8)) +
         data.feeders_[0].distribution_.probability_greater(
             timestamp_to_delay(data.feeders_[0].arrival_time_, 6 * 60 + 9)));
    // feeder1 = 06:08 * feeder2 = 06:08
    probability const prob_both_feeders =
        data.feeders_[0].distribution_.probability_equal(
            timestamp_to_delay(data.feeders_[0].arrival_time_, 6 * 60 + 8)) *
        data.feeders_[1].distribution_.probability_equal(
            timestamp_to_delay(data.feeders_[1].arrival_time_, 6 * 60 + 8));

    REQUIRE(equal(
        had_to_wait_for_feeders(data.feeders_, modified_feeders_distributions,
                                data.scheduled_departure_time_ + 2),
        prob_feeder1 + prob_feeder2 + prob_both_feeders));
  }

  // probability 2=0.0439064
  REQUIRE(equal(departure_distribution.probability_equal(2), 0.0439064));
  REQUIRE(equal(
      departure_distribution.probability_equal(2),
      (departure_independent_from_feeders(data.feeders_,
                                          data.scheduled_departure_time_ + 2) *
           train_arrives_at_time(data, data.scheduled_departure_time_ + 2) +
       (train_arrives_before_time(data, data.scheduled_departure_time_ + 2) *
        had_to_wait_for_feeders(data.feeders_, modified_feeders_distributions,
                                data.scheduled_departure_time_ + 2)))));

  /******************* minute 3 *******************/

  REQUIRE(equal(departure_independent_from_feeders(
                    data.feeders_, data.scheduled_departure_time_ + 3),
                1.0));
  REQUIRE(equal(train_arrives_at_time(data, data.scheduled_departure_time_ + 3),
                0.033));
  REQUIRE(
      equal(train_arrives_before_time(data, data.scheduled_departure_time_ + 3),
            0.043 + 6 * 0.033));

  {
    // feeder1 = 06:09 * (feeder2 < 06:09 + feeder2 > 06:09)
    probability const prob_feeder1 =
        data.feeders_[0].distribution_.probability_equal(
            timestamp_to_delay(data.feeders_[0].arrival_time_, 6 * 60 + 9)) *
        (data.feeders_[1].distribution_.probability_smaller(
             timestamp_to_delay(data.feeders_[1].arrival_time_, 6 * 60 + 9)) +
         data.feeders_[1].distribution_.probability_greater(
             timestamp_to_delay(data.feeders_[1].arrival_time_, 6 * 60 + 9)));
    // feeder2 = 06:09 * (feeder1 < 06:09 + feeder1 > 06:09)
    probability const prob_feeder2 =
        data.feeders_[1].distribution_.probability_equal(
            timestamp_to_delay(data.feeders_[1].arrival_time_, 6 * 60 + 9)) *
        (data.feeders_[0].distribution_.probability_smaller(
             timestamp_to_delay(data.feeders_[0].arrival_time_, 6 * 60 + 9)) +
         data.feeders_[0].distribution_.probability_greater(
             timestamp_to_delay(data.feeders_[0].arrival_time_, 6 * 60 + 9)));
    // feeder1 = 06:09 * feeder2 = 06:09
    probability const prob_both_feeders =
        data.feeders_[0].distribution_.probability_equal(
            timestamp_to_delay(data.feeders_[0].arrival_time_, 6 * 60 + 9)) *
        data.feeders_[1].distribution_.probability_equal(
            timestamp_to_delay(data.feeders_[1].arrival_time_, 6 * 60 + 9));

    REQUIRE(equal(
        had_to_wait_for_feeders(data.feeders_, modified_feeders_distributions,
                                data.scheduled_departure_time_ + 3),
        prob_feeder1 + prob_feeder2 + prob_both_feeders));
  }

  // probability 3=0.0486436
  REQUIRE(equal(departure_distribution.probability_equal(3), 0.0486436));
  REQUIRE(equal(
      departure_distribution.probability_equal(3),
      (departure_independent_from_feeders(data.feeders_,
                                          data.scheduled_departure_time_ + 3) *
           train_arrives_at_time(data, data.scheduled_departure_time_ + 3) +
       (train_arrives_before_time(data, data.scheduled_departure_time_ + 3) *
        had_to_wait_for_feeders(data.feeders_, modified_feeders_distributions,
                                data.scheduled_departure_time_ + 3)))));

  /******************* minutes 4-25 *******************/

  for (unsigned int d = 4; d <= 25; d++) {
    REQUIRE(
        equal(train_arrives_at_time(data, data.scheduled_departure_time_ + d),
              0.033));
    REQUIRE(equal(departure_distribution.probability_equal(d), 0.033));
  }
}
