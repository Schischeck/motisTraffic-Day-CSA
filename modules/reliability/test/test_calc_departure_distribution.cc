#include "catch/catch.hpp"

#include <iostream>

#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

#include "motis/loader/loader.h"

#include "motis/reliability/calc_departure_distribution.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/pd_calc_data_departure.h"
#include "motis/reliability/probability_distribution.h"
#include "motis/reliability/train_distributions.h"

#include "include/tt_distributions_test_manager.h"

using namespace motis;
using namespace motis::reliability;
using namespace motis::reliability::calc_departure_distribution;
using namespace motis::reliability::calc_departure_distribution::detail;

TEST_CASE("departure_independent_from_feeders", "[calc_departure_distribution]") {
  std::vector<pd_calc_data_departure::feeder_info> feeders;
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

TEST_CASE("cut_minutes_after_latest_feasible_arrival1",
          "[calc_departure_distribution]") {
  probability_distribution feeder1_dist;
  std::vector<pd_calc_data_departure::feeder_info> feeders;
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
  std::vector<pd_calc_data_departure::feeder_info> feeders;
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

TEST_CASE("had_to_wait_for_feeders1", "[pd_calc_data_departure]") {
  probability_distribution feeder1_dist;
  std::vector<pd_calc_data_departure::feeder_info> feeders;
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
TEST_CASE("had_to_wait_for_feeders2", "[pd_calc_data_departure]") {
  probability_distribution feeder1_dist;
  std::vector<pd_calc_data_departure::feeder_info> feeders;
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
TEST_CASE("departure_at_scheduled_time1", "[calc_departure_distribution]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");

  train_distributions_container const* const train_distributions = nullptr;
  tt_distributions_test_manager tt_distributions({0.6, 0.4});

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node = *schedule->route_index_to_first_route_node[4];
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& first_light_conn = first_route_edge->_m._route_edge._conns[0];

  pd_calc_data_departure data(first_route_node, first_light_conn, true,
                              *schedule, *train_distributions, tt_distributions);

  REQUIRE(equal(departure_at_scheduled_time(data), 0.6));
}

// route node with feeders
TEST_CASE("departure_at_scheduled_time2", "[calc_departure_distribution]") {}

TEST_CASE("departure_within_waiting_interval", "[calc_departure_distribution]") {}

// first route node
TEST_CASE("departure_after_waiting_interval1", "[calc_departure_distribution]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");

  train_distributions_container const* const train_distributions = nullptr;
  tt_distributions_test_manager tt_distributions({0.6, 0.4});

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node = *schedule->route_index_to_first_route_node[4];
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& first_light_conn = first_route_edge->_m._route_edge._conns[0];

  pd_calc_data_departure data(first_route_node, first_light_conn, true,
                              *schedule, *train_distributions, tt_distributions);

  REQUIRE(equal(departure_after_waiting_interval(data, 5 * 60 + 56), 0.4));
}

// route node with preceding arrival
TEST_CASE("departure_after_waiting_interval2", "[calc_departure_distribution]") {}

// first route node with no feeders
TEST_CASE("compute_departure_distribution1", "[calc_departure_distribution]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");

  train_distributions_container const* const train_distributions = nullptr;
  tt_distributions_test_manager tt_distributions({0.6, 0.4});

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node = *schedule->route_index_to_first_route_node[4];
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& first_light_conn = first_route_edge->_m._route_edge._conns[0];

  pd_calc_data_departure data(first_route_node, first_light_conn, true,
                              *schedule, *train_distributions, tt_distributions);
  probability_distribution departure_distribution;
  compute_departure_distribution(data, departure_distribution);

  REQUIRE(departure_distribution.first_minute() == 0);
  REQUIRE(departure_distribution.last_minute() == 1);
  REQUIRE(equal(departure_distribution.sum(), 1.0));
  REQUIRE(equal(departure_distribution.probability_equal(0), 0.6));
  REQUIRE(equal(departure_distribution.probability_equal(1), 0.4));
}
