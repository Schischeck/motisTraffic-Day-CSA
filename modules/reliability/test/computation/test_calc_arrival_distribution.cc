#include "catch/catch.hpp"

#include <iostream>

#include "motis/core/common/date_util.h"

#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

#include "motis/loader/loader.h"

#include "motis/reliability/distributions_container.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/probability_distribution.h"
#include "motis/reliability/computation/calc_arrival_distribution.h"
#include "motis/reliability/computation/data_arrival.h"

#include "include/precomputed_distributions_test_container.h"
#include "include/start_and_travel_test_distributions.h"

using namespace motis;
using namespace motis::reliability;
using namespace motis::reliability::calc_arrival_distribution;
using namespace motis::reliability::calc_arrival_distribution::detail;

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

TEST_CASE("correct_rounding_errors", "[calc_arrival_distribution]") {
  std::vector<probability> probabilities = {0.1, 0.5, 0.25, 0.15};

  correct_rounding_errors(1.0, probabilities);

  REQUIRE(equal(probabilities[0], 0.1));
  REQUIRE(equal(probabilities[1], 0.5));
  REQUIRE(equal(probabilities[2], 0.25));
  REQUIRE(equal(probabilities[3], 0.15));

  probabilities[0] = 0.09999;
  correct_rounding_errors(1.0, probabilities);

  REQUIRE(equal(probabilities[0], 0.09999));
  REQUIRE(equal(probabilities[1], 0.50001));
  REQUIRE(equal(probabilities[2], 0.25));
  REQUIRE(equal(probabilities[3], 0.15));
}

TEST_CASE("compute_arrival_distribution", "[calc_arrival_distribution]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));

  probability_distribution dep_dist;
  dep_dist.init({0.8, 0.2}, 0);
  start_and_travel_test_distributions s_t_distributions({0.1, 0.7, 0.2}, -1);

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule, schedule1::ICE_FR_DA_H);
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& light_connection = first_route_edge->_m._route_edge._conns[0];
  auto const& second_route_node = *first_route_edge->_to;

  data_arrival data(second_route_node, light_connection, dep_dist, *schedule,
                    s_t_distributions);
  probability_distribution arrival_distribution;

  compute_arrival_distribution(data, arrival_distribution);

  REQUIRE(arrival_distribution.first_minute() == -1);
  REQUIRE(arrival_distribution.last_minute() == 2);
  REQUIRE(equal(arrival_distribution.sum(), 1.0));
  REQUIRE(equal(arrival_distribution.probability_equal(-1), 0.8 * 0.1));
  REQUIRE(
      equal(arrival_distribution.probability_equal(0), 0.8 * 0.7 + 0.2 * 0.1));
  REQUIRE(
      equal(arrival_distribution.probability_equal(1), 0.8 * 0.2 + 0.2 * 0.7));
  REQUIRE(equal(arrival_distribution.probability_equal(2), 0.2 * 0.2));
}

/* sum lower than 1.0 */
TEST_CASE("compute_arrival_distribution2", "[calc_arrival_distribution]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));

  probability_distribution dep_dist;
  dep_dist.init({0.7, 0.15}, 0);
  start_and_travel_test_distributions s_t_distributions({0.1, 0.7, 0.2}, -1);

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule, schedule1::ICE_FR_DA_H);
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& light_connection = first_route_edge->_m._route_edge._conns[0];
  auto const& second_route_node = *first_route_edge->_to;

  data_arrival data(second_route_node, light_connection, dep_dist, *schedule,
                    s_t_distributions);
  probability_distribution arrival_distribution;

  compute_arrival_distribution(data, arrival_distribution);

  REQUIRE(arrival_distribution.first_minute() == -1);
  REQUIRE(arrival_distribution.last_minute() == 2);
  REQUIRE(equal(arrival_distribution.sum(), 0.85));
  REQUIRE(equal(arrival_distribution.probability_equal(-1), 0.7 * 0.1));
  REQUIRE(
      equal(arrival_distribution.probability_equal(0), 0.7 * 0.7 + 0.15 * 0.1));
  REQUIRE(
      equal(arrival_distribution.probability_equal(1), 0.7 * 0.2 + 0.15 * 0.7));
  REQUIRE(equal(arrival_distribution.probability_equal(2), 0.15 * 0.2));
}

/* arrival distribution with a gap */
TEST_CASE("compute_arrival_distribution3", "[calc_arrival_distribution]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));

  probability_distribution dep_dist;
  dep_dist.init({0.5, 0.0, 0.5}, 0);
  start_and_travel_test_distributions s_t_distributions({1.0}, 0);

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule, schedule1::ICE_FR_DA_H);
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& light_connection = first_route_edge->_m._route_edge._conns[0];
  auto const& second_route_node = *first_route_edge->_to;

  data_arrival data(second_route_node, light_connection, dep_dist, *schedule,
                    s_t_distributions);
  probability_distribution arrival_distribution;

  compute_arrival_distribution(data, arrival_distribution);

  REQUIRE(arrival_distribution.first_minute() == 0);
  REQUIRE(arrival_distribution.last_minute() == 2);
  REQUIRE(equal(arrival_distribution.sum(), 1.0));
  REQUIRE(equal(arrival_distribution.probability_equal(0), 0.5));
  REQUIRE(equal(arrival_distribution.probability_equal(1), 0.0));
  REQUIRE(equal(arrival_distribution.probability_equal(2), 0.5));
}
