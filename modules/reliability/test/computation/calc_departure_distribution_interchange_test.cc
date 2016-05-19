#include "gtest/gtest.h"

#include <iostream>

#include "motis/core/common/date_time_util.h"

#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

#include "motis/loader/loader.h"

#include "motis/reliability/computation/calc_departure_distribution_interchange.h"
#include "motis/reliability/computation/data_departure_interchange.h"
#include "motis/reliability/distributions/probability_distribution.h"

#include "../include/interchange_data_for_tests.h"
#include "../include/schedules/schedule2.h"
#include "../include/start_and_travel_test_distributions.h"
#include "../include/test_container.h"
#include "../include/test_schedule_setup.h"
#include "../include/test_util.h"

namespace motis {
namespace reliability {
namespace calc_departure_distribution {

class reliability_calc_departure_distribution2 : public test_schedule_setup {
public:
  reliability_calc_departure_distribution2()
      : test_schedule_setup(schedule2::PATH, schedule2::DATE) {}
};

TEST_F(reliability_calc_departure_distribution2, ic_feeder_arrived) {
  probability_distribution pd;
  pd.init({.1, .1, .1, .1, .1, .1, .1, .1}, 0);  // sum = 0.8

  data_departure_interchange::interchange_feeder_info info;
  info.scheduled_arrival_time_ = 0;
  info.transfer_time_ = 4;
  info.waiting_time_ = 3;
  info.latest_feasible_arrival_ = 5;
  info.arrival_distribution_ = &pd;

  for (unsigned int i = 0; i < 8; i++) {
    probability const prob = interchange::detail::ic_feeder_arrived(
        info, static_cast<motis::time>(4 + i));
    ASSERT_TRUE(equal(prob, (i + 1) * 0.1));
  }
}

TEST_F(reliability_calc_departure_distribution2, ic_feeder_arrives_at_time) {
  probability_distribution pd;
  pd.init({.1, .1, .1, .1, .1, .1, .1, .1}, 0);  // sum = 0.8

  data_departure_interchange::interchange_feeder_info info;
  info.scheduled_arrival_time_ = 0;
  info.transfer_time_ = 4;
  info.waiting_time_ = 3;
  info.latest_feasible_arrival_ = 5;
  info.arrival_distribution_ = &pd;

  for (unsigned int i = 0; i < 8; i++) {
    probability const prob = interchange::detail::ic_feeder_arrives_at_time(
        info, static_cast<motis::time>(4 + i));
    ASSERT_TRUE(equal(prob, 0.1));
  }
}

// first route node without feeders, no waiting, interchange highly reliable
TEST_F(reliability_calc_departure_distribution2,
       compute_departure_distribution_ic1) {
  interchange_data_for_tests const ic_data(
      sched(), schedule2::RE_K_F, schedule2::ICE_F_S, schedule2::KASSEL.eva_,
      schedule2::FRANKFURT.eva_, schedule2::STUTTGART.eva_, 8 * 60, 10 * 60,
      10 * 60 + 10, 11 * 60 + 10);
  distributions_container::container dummy;
  distributions_container::container::node dummy_node;
  start_and_travel_test_distributions s_t_distributions({.6, .4});
  probability_distribution arrival_distribution;
  arrival_distribution.init({.1, .7, .2}, -1);
  data_departure_interchange data(
      true, ic_data.tail_node_departing_train_,
      *ic_data.arriving_route_edge_.to_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, arrival_distribution, dummy, dummy_node,
      context(sched(), dummy, s_t_distributions));
  ASSERT_TRUE(data.interchange_feeder_info_.transfer_time_ == 5);
  ASSERT_TRUE(data.interchange_feeder_info_.waiting_time_ == 0);

  probability_distribution departure_distribution;
  interchange::compute_departure_distribution(data, departure_distribution);

  ASSERT_TRUE(departure_distribution.first_minute() == 0);
  ASSERT_TRUE(departure_distribution.last_minute() == 1);
  ASSERT_TRUE(equal(departure_distribution.sum(), 1.0));
  ASSERT_TRUE(equal(departure_distribution.probability_equal(0), .6));
  ASSERT_TRUE(equal(departure_distribution.probability_equal(1), .4));
}

// first route node without feeders, no waiting, interchange unreliable
TEST_F(reliability_calc_departure_distribution2,
       compute_departure_distribution_ic2) {
  interchange_data_for_tests const ic_data(
      sched(), schedule2::RE_K_F, schedule2::ICE_F_S, schedule2::KASSEL.eva_,
      schedule2::FRANKFURT.eva_, schedule2::STUTTGART.eva_, 8 * 60, 10 * 60,
      10 * 60 + 10, 11 * 60 + 10);
  distributions_container::container dummy;
  distributions_container::container::node dummy_node;
  start_and_travel_test_distributions s_t_distributions({.6, .4});
  probability_distribution arrival_distribution;
  arrival_distribution.init({.05, .05, .1, .1, .1, .1, .1, .1, 0.3}, -1);

  data_departure_interchange data(
      true, ic_data.tail_node_departing_train_,
      *ic_data.arriving_route_edge_.to_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, arrival_distribution, dummy, dummy_node,
      context(sched(), dummy, s_t_distributions));
  ASSERT_TRUE(data.interchange_feeder_info_.transfer_time_ == 5);
  ASSERT_TRUE(data.interchange_feeder_info_.waiting_time_ == 0);

  probability_distribution departure_distribution;
  interchange::compute_departure_distribution(data, departure_distribution);

  ASSERT_TRUE(departure_distribution.first_minute() == 0);
  ASSERT_TRUE(departure_distribution.last_minute() == 1);
  ASSERT_TRUE(
      equal(departure_distribution.probability_equal(0), .6 * .6));  // 10:10
  ASSERT_TRUE(
      equal(departure_distribution.probability_equal(1), .4 * .7));  // 10:11
}

// preceding arrival, no other feeders, no waiting for ic feeder
TEST_F(reliability_calc_departure_distribution2,
       compute_departure_distribution_ic3) {
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});
  // route node at Frankfurt of train ICE_K_F_S
  auto& tail_node_departing_train =
      *graph_accessor::get_departing_route_edge(
           *graph_accessor::get_first_route_node(sched(), schedule2::ICE_K_F_S))
           ->to_;
  ASSERT_EQ(
      schedule2::FRANKFURT.eva_,
      sched().stations_[tail_node_departing_train.station_node_->id_]->eva_nr_);
  probability_distribution arrival_distribution;
  arrival_distribution.init({.4, .3, .2, .1}, 14);  // 10:14 - 10:17

  // arriving train RE_K_F from Kassel to Frankfurt
  // interchange at Frankfurt (second node of ICE_K_F_S)
  // departing train ICE_K_F_S from Frankfurt to Stuttgart
  interchange_data_for_tests const ic_data(
      sched(), schedule2::RE_K_F, tail_node_departing_train,
      schedule2::KASSEL.eva_, schedule2::FRANKFURT.eva_,
      schedule2::STUTTGART.eva_, 8 * 60, 10 * 60, 10 * 60 + 20, 11 * 60 + 15);

  // preceding-arrival: 10:18 - 10:19
  distributions_container::container precomputed;
  auto const& distribution_node = init_feeders_and_get_distribution_node(
      precomputed, tail_node_departing_train, ic_data.departing_light_conn_,
      {0.9, 0.1}, 3, sched());

  probability_distribution preceding_arr_dist;
  preceding_arr_dist.init({0.9, 0.1}, 3);
  distributions_container::single_distribution_container
      preceding_arrival_container(preceding_arr_dist);

  data_departure_interchange data(
      false, tail_node_departing_train, *ic_data.arriving_route_edge_.to_,
      ic_data.departing_light_conn_, ic_data.arriving_light_conn_,
      arrival_distribution, preceding_arrival_container, distribution_node,
      context(sched(), precomputed, s_t_distributions));

  ASSERT_EQ(5, data.interchange_feeder_info_.transfer_time_);
  ASSERT_EQ(0, data.interchange_feeder_info_.waiting_time_);
  ASSERT_EQ(test_util::minutes_to_motis_time(10 * 60 + 15),
            data.train_info_.preceding_arrival_info_.scheduled_arrival_time_);
  ASSERT_EQ(2, data.train_info_.preceding_arrival_info_.min_standing_);

  probability_distribution departure_distribution;
  interchange::compute_departure_distribution(data, departure_distribution);

  ASSERT_EQ(0, departure_distribution.first_minute());
  ASSERT_EQ(1, departure_distribution.last_minute());
  ASSERT_TRUE(equal(.9 * .7,
                    departure_distribution.probability_equal(0)));  // 10:20
  ASSERT_TRUE(equal(.1 * .9,
                    departure_distribution.probability_equal(1)));  // 10:21
}

// first route node, waiting for ic-feeder and other feeder
TEST_F(reliability_calc_departure_distribution2,
       compute_departure_distribution_ic5) {
  start_and_travel_test_distributions s_t_distributions(
      {.6, .4});  // start distribution 11:32 - 11:33
  // arriving train ICE_F_S from Frankfurt to Stuttgart
  // interchange at Stuttgart
  // departing train ICE_S_E from Stuttgart to Erlangen
  interchange_data_for_tests const ic_data(
      sched(), schedule2::ICE_F_S, schedule2::ICE_S_E,
      schedule2::FRANKFURT.eva_, schedule2::STUTTGART.eva_,
      schedule2::ERLANGEN.eva_, 10 * 60 + 10, 11 * 60 + 10, 11 * 60 + 32,
      12 * 60 + 32);
  probability_distribution arrival_distribution;
  arrival_distribution.init({.4, .3, .2, .1}, 16);  // 11:26 - 11:29

  // feeder arrival 11:27 - 11:29
  distributions_container::container precomputed;
  auto const& distribution_node = init_feeders_and_get_distribution_node(
      precomputed, ic_data.tail_node_departing_train_,
      ic_data.departing_light_conn_, {.9, .05, .05}, 12, sched());

  data_departure_interchange data(
      true, ic_data.tail_node_departing_train_,
      *ic_data.arriving_route_edge_.to_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, arrival_distribution, precomputed,
      distribution_node, context(sched(), precomputed, s_t_distributions));

  ASSERT_TRUE(data.interchange_feeder_info_.transfer_time_ == 5);
  ASSERT_TRUE(data.interchange_feeder_info_.waiting_time_ == 3);
  ASSERT_TRUE(data.feeders_.at(0).scheduled_arrival_time_ ==
              test_util::minutes_to_motis_time(11 * 60 + 15));
  ASSERT_TRUE(data.feeders_.at(0).transfer_time_ == 5);
  ASSERT_TRUE(data.feeders_.at(0).latest_feasible_arrival_ ==
              test_util::minutes_to_motis_time(11 * 60 + 30));

  probability_distribution departure_distribution;
  interchange::compute_departure_distribution(data, departure_distribution);

  ASSERT_TRUE(departure_distribution.first_minute() == 0);
  ASSERT_TRUE(departure_distribution.last_minute() == 2);
  ASSERT_TRUE(equal(departure_distribution.probability_equal(0),
                    .6 * .9 * .7));  // 11:32
  probability const prob1 =
      (.4 * .95 * .9) /* train departs with delay at start */
      + (.6 * .05 * .2) /* waits for ic feeder and other feeder */
      + (.6 * .9 * .2) /* waits only for ic feeder */
      + (.6 * .05 * .7) /* waits only for other feeder */;
  ASSERT_TRUE(
      equal(departure_distribution.probability_equal(1), prob1));  // 11:33
  probability const prob2 =
      (.05 * .1) /* waits for ic feeder and other feeder */
      + (.95 * .1) /* waits only for ic feeder */
      + (.05 * .9) /* waits only for other feeder */;
  ASSERT_TRUE(
      equal(departure_distribution.probability_equal(2), prob2));  // 11:34
}

}  // namespace calc_departure_distribution
}  // namespace reliability
}  // namespace motis
