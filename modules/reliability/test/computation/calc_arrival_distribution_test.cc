#include "gtest/gtest.h"

#include <iostream>

#include "motis/core/common/date_time_util.h"
#include "motis/core/schedule/time.h"

#include "motis/reliability/computation/calc_arrival_distribution.h"
#include "motis/reliability/computation/data_arrival.h"
#include "motis/reliability/distributions/distributions_container.h"
#include "motis/reliability/distributions/probability_distribution.h"
#include "motis/reliability/graph_accessor.h"

#include "../include/schedules/schedule1.h"
#include "../include/start_and_travel_test_distributions.h"
#include "../include/test_container.h"
#include "../include/test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace calc_arrival_distribution {
using namespace detail;

class reliability_calc_arrival_distribution : public test_schedule_setup {
public:
  reliability_calc_arrival_distribution()
      : test_schedule_setup(schedule1::PATH, schedule1::DATE) {}
};

TEST_F(reliability_calc_arrival_distribution, correct_rounding_errors) {
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

TEST_F(reliability_calc_arrival_distribution, compute_arrival_distribution) {
  probability_distribution dep_dist;
  dep_dist.init({0.8, 0.2}, 0);
  start_and_travel_test_distributions s_t_distributions({0.1, 0.7, 0.2}, -1);

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node =
      *graph_accessor::get_first_route_node(sched(), schedule1::ICE_FR_DA_H);
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& light_connection = first_route_edge->m_.route_edge_.conns_[0];

  data_arrival data(*first_route_edge->from_, *first_route_edge->to_,
                    light_connection, dep_dist, sched(), s_t_distributions);
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
TEST_F(reliability_calc_arrival_distribution, compute_arrival_distribution2) {
  probability_distribution dep_dist;
  dep_dist.init({0.7, 0.15}, 0);
  start_and_travel_test_distributions s_t_distributions({0.1, 0.7, 0.2}, -1);

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node =
      *graph_accessor::get_first_route_node(sched(), schedule1::ICE_FR_DA_H);
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& light_connection = first_route_edge->m_.route_edge_.conns_[0];

  data_arrival data(*first_route_edge->from_, *first_route_edge->to_,
                    light_connection, dep_dist, sched(), s_t_distributions);
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
TEST_F(reliability_calc_arrival_distribution, compute_arrival_distribution3) {
  probability_distribution dep_dist;
  dep_dist.init({0.5, 0.0, 0.5}, 0);
  start_and_travel_test_distributions s_t_distributions({1.0}, 0);

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node =
      *graph_accessor::get_first_route_node(sched(), schedule1::ICE_FR_DA_H);
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& light_connection = first_route_edge->m_.route_edge_.conns_[0];

  data_arrival data(*first_route_edge->from_, *first_route_edge->to_,
                    light_connection, dep_dist, sched(), s_t_distributions);
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
