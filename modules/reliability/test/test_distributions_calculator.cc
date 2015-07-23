#include "catch/catch.hpp"

#include <iostream>

#include "motis/core/schedule/time.h"

#include "motis/reliability/distributions_calculator.h"
#include "motis/reliability/pd_calc_data_departure.h"
#include "motis/reliability/probability_distribution.h"

using namespace motis;
using namespace motis::reliability;
using namespace motis::reliability::distributions_calculator;
using namespace motis::reliability::distributions_calculator::detail;

TEST_CASE("get_prob_no_waiting_for_feeders", "[distributions_calculator]") {
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

TEST_CASE("cut_minutes_after_latest_feasible_arrival1", "[distributions_calculator]") {
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

TEST_CASE("cut_minutes_after_latest_feasible_arrival2", "[distributions_calculator]") {
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
