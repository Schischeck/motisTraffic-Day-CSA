#include "gtest/gtest.h"

#include <iostream>

#include "motis/core/common/date_util.h"
#include "motis/core/schedule/time.h"

#include "motis/reliability/distributions_container.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/probability_distribution.h"
#include "motis/reliability/computation/calc_arrival_distribution.h"
#include "motis/reliability/computation/data_arrival.h"

#include "include/precomputed_distributions_test_container.h"
#include "include/start_and_travel_test_distributions.h"
#include "include/test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace calc_arrival_distribution {
using namespace detail;

class test_calc_arrival_distribution : public test_schedule_setup {
public:
  test_calc_arrival_distribution()
      : test_schedule_setup("modules/reliability/resources/schedule/",
                            to_unix_time(2015, 9, 28),
                            to_unix_time(2015, 9, 29)) {}
  /* train numbers */
  short const IC_DA_H = 1;
  short const IC_FR_DA = 2;
  short const IC_FH_DA = 3;
  short const RE_MA_DA = 4;
  short const ICE_FR_DA_H = 5;
  short const ICE_HA_W_HE = 6;
  short const ICE_K_K = 7;
  short const RE_K_S = 8;
};

TEST_F(test_calc_arrival_distribution, correct_rounding_errors) {
  std::vector<probability> probabilities = {0.1, 0.5, 0.25, 0.15};

  correct_rounding_errors(1.0, probabilities);

  ASSERT_TRUE(equal(probabilities[0], 0.1));
  ASSERT_TRUE(equal(probabilities[1], 0.5));
  ASSERT_TRUE(equal(probabilities[2], 0.25));
  ASSERT_TRUE(equal(probabilities[3], 0.15));

  probabilities[0] = 0.09999;
  correct_rounding_errors(1.0, probabilities);

  ASSERT_TRUE(equal(probabilities[0], 0.09999));
  ASSERT_TRUE(equal(probabilities[1], 0.50001));
  ASSERT_TRUE(equal(probabilities[2], 0.25));
  ASSERT_TRUE(equal(probabilities[3], 0.15));
}

TEST_F(test_calc_arrival_distribution, compute_arrival_distribution) {
  probability_distribution dep_dist;
  dep_dist.init({0.8, 0.2}, 0);
  start_and_travel_test_distributions s_t_distributions({0.1, 0.7, 0.2}, -1);

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule_, ICE_FR_DA_H);
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& light_connection = first_route_edge->_m._route_edge._conns[0];
  auto const& second_route_node = *first_route_edge->_to;

  data_arrival data(second_route_node, light_connection, dep_dist, *schedule_,
                    s_t_distributions);
  probability_distribution arrival_distribution;

  compute_arrival_distribution(data, arrival_distribution);

  ASSERT_EQ(arrival_distribution.first_minute(), -1);
  ASSERT_EQ(arrival_distribution.last_minute(), 2);
  ASSERT_TRUE(equal(arrival_distribution.sum(), 1.0));
  ASSERT_TRUE(equal(arrival_distribution.probability_equal(-1), 0.8 * 0.1));
  ASSERT_TRUE(
      equal(arrival_distribution.probability_equal(0), 0.8 * 0.7 + 0.2 * 0.1));
  ASSERT_TRUE(
      equal(arrival_distribution.probability_equal(1), 0.8 * 0.2 + 0.2 * 0.7));
  ASSERT_TRUE(equal(arrival_distribution.probability_equal(2), 0.2 * 0.2));
}

/* sum lower than 1.0 */
TEST_F(test_calc_arrival_distribution, compute_arrival_distribution2) {
  probability_distribution dep_dist;
  dep_dist.init({0.7, 0.15}, 0);
  start_and_travel_test_distributions s_t_distributions({0.1, 0.7, 0.2}, -1);

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule_, ICE_FR_DA_H);
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& light_connection = first_route_edge->_m._route_edge._conns[0];
  auto const& second_route_node = *first_route_edge->_to;

  data_arrival data(second_route_node, light_connection, dep_dist, *schedule_,
                    s_t_distributions);
  probability_distribution arrival_distribution;

  compute_arrival_distribution(data, arrival_distribution);

  ASSERT_TRUE(arrival_distribution.first_minute() == -1);
  ASSERT_TRUE(arrival_distribution.last_minute() == 2);
  ASSERT_TRUE(equal(arrival_distribution.sum(), 0.85));
  ASSERT_TRUE(equal(arrival_distribution.probability_equal(-1), 0.7 * 0.1));
  ASSERT_TRUE(
      equal(arrival_distribution.probability_equal(0), 0.7 * 0.7 + 0.15 * 0.1));
  ASSERT_TRUE(
      equal(arrival_distribution.probability_equal(1), 0.7 * 0.2 + 0.15 * 0.7));
  ASSERT_TRUE(equal(arrival_distribution.probability_equal(2), 0.15 * 0.2));
}

/* arrival distribution with a gap */
TEST_F(test_calc_arrival_distribution, compute_arrival_distribution3) {
  probability_distribution dep_dist;
  dep_dist.init({0.5, 0.0, 0.5}, 0);
  start_and_travel_test_distributions s_t_distributions({1.0}, 0);

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule_, ICE_FR_DA_H);
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& light_connection = first_route_edge->_m._route_edge._conns[0];
  auto const& second_route_node = *first_route_edge->_to;

  data_arrival data(second_route_node, light_connection, dep_dist, *schedule_,
                    s_t_distributions);
  probability_distribution arrival_distribution;

  compute_arrival_distribution(data, arrival_distribution);

  ASSERT_TRUE(arrival_distribution.first_minute() == 0);
  ASSERT_TRUE(arrival_distribution.last_minute() == 2);
  ASSERT_TRUE(equal(arrival_distribution.sum(), 1.0));
  ASSERT_TRUE(equal(arrival_distribution.probability_equal(0), 0.5));
  ASSERT_TRUE(equal(arrival_distribution.probability_equal(1), 0.0));
  ASSERT_TRUE(equal(arrival_distribution.probability_equal(2), 0.5));
}

}  // namespace calc_arrival_distribution
}  // namespace reliability
}  // namespace motis
