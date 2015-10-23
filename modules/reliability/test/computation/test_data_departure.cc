#include "gtest/gtest.h"

#include "motis/loader/loader.h"

#include "motis/core/common/date_util.h"

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/distributions_container.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/computation/data_arrival.h"
#include "motis/reliability/computation/data_departure.h"

#include "include/precomputed_distributions_test_container.h"
#include "include/start_and_travel_test_distributions.h"

using namespace motis;
using namespace motis::reliability;
using namespace motis::reliability::calc_departure_distribution;

namespace schedule1 {
/* eva numbers */
std::string const DARMSTADT = "4219971";
std::string const FRANKFURT = "8351230";
std::string const KARLSRUHE = "7226036";
std::string const WURZBURG = "0064254";
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

TEST(first_route_node_no_feeders, pd_calc_data_departure) {
  auto schedule = loader::load_schedule(
      "modules/reliability/resources/schedule/", to_unix_time(2015, 9, 28),
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

  ASSERT_TRUE(schedule->stations[first_route_node._station_node->_id]->eva_nr ==
              schedule1::FRANKFURT);
  ASSERT_TRUE(first_light_conn.d_time == 5 * 60 + 55);
  ASSERT_TRUE(first_light_conn.a_time == 6 * 60 + 5);
  ASSERT_TRUE(first_light_conn._full_con->con_info->train_nr == 5);

  ASSERT_TRUE(data.scheduled_departure_time_ == first_light_conn.d_time);
  ASSERT_TRUE(data.largest_delay() == 1);
  ASSERT_TRUE(data.maximum_waiting_time_ == 0);
  ASSERT_TRUE(data.is_first_route_node_);

  auto const& start_distribution =
      data.train_info_.first_departure_distribution_;
  ASSERT_TRUE(equal(start_distribution->sum(), 1.0));
  ASSERT_TRUE(start_distribution->first_minute() == 0);
  ASSERT_TRUE(start_distribution->last_minute() == 1);
  ASSERT_TRUE(equal(start_distribution->probability_equal(0), 0.6));
  ASSERT_TRUE(equal(start_distribution->probability_equal(1), 0.4));

  ASSERT_TRUE(data.feeders_.size() == 0);
}

TEST(preceding_arrival_no_feeders, pd_calc_data_departure) {
  auto schedule = loader::load_schedule(
      "modules/reliability/resources/schedule/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));

  precomputed_distributions_test_container train_distributions({0.1, 0.7, 0.2},
                                                               -1);
  distributions_container::precomputed_distributions_container dummy(0);
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
                      train_distributions, dummy, s_t_distributions);

  ASSERT_TRUE(
      schedule->stations[second_route_node->_station_node->_id]->eva_nr ==
      schedule1::WURZBURG);
  ASSERT_TRUE(light_connection.d_time == 10 * 60 + 34);
  ASSERT_TRUE(light_connection.a_time == 11 * 60 + 7);

  ASSERT_TRUE(data.scheduled_departure_time_ == light_connection.d_time);
  ASSERT_TRUE(data.largest_delay() == 1);
  ASSERT_TRUE(!data.is_first_route_node_);
  ASSERT_TRUE(data.maximum_waiting_time_ == 0);
  ASSERT_TRUE(data.feeders_.size() == 0);

  ASSERT_TRUE(data.train_info_.preceding_arrival_info_.arrival_time_ ==
              10 * 60 + 32);
  ASSERT_TRUE(data.train_info_.preceding_arrival_info_.min_standing_ == 2);
  ASSERT_TRUE(data.train_info_.preceding_arrival_info_.arrival_distribution_ ==
              &train_distributions.get_distribution(
                  0, 0, distributions_container::arrival));
}

TEST(first_route_node_feeders, pd_calc_data_departure) {
  auto schedule = loader::load_schedule(
      "modules/reliability/resources/schedule/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));

  distributions_container::precomputed_distributions_container dummy(0);
  precomputed_distributions_test_container feeder_distributions({0.1, 0.7, 0.2},
                                                                -1);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route node at Darmstadt of train IC_DA_H
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule, schedule1::IC_DA_H);
  // route edge from Darmstadt to Heidelberg
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  // light connection d07:00 a07:28
  auto const& light_connection = first_route_edge->_m._route_edge._conns[1];

  data_departure data(first_route_node, light_connection, true, *schedule,
                      dummy, feeder_distributions, s_t_distributions);

  ASSERT_TRUE(schedule->stations[first_route_node._station_node->_id]->eva_nr ==
              schedule1::DARMSTADT);
  ASSERT_TRUE(light_connection.d_time == 7 * 60);
  ASSERT_TRUE(light_connection.a_time == 7 * 60 + 28);

  ASSERT_TRUE(data.scheduled_departure_time_ == light_connection.d_time);
  ASSERT_TRUE(data.largest_delay() == data.maximum_waiting_time_);
  ASSERT_TRUE(data.is_first_route_node_);

  ASSERT_TRUE(data.train_info_.first_departure_distribution_ ==
              &s_t_distributions.get_start_distribution("dummy"));

  ASSERT_TRUE(data.maximum_waiting_time_ == 3);
  ASSERT_TRUE(data.feeders_.size() == 2);

  ASSERT_TRUE(data.feeders_[0].arrival_time_ == 6 * 60 + 54);
  ASSERT_TRUE(data.feeders_[0].transfer_time_ ==
              5);  // TODO use platform change time
  ASSERT_TRUE(data.feeders_[0].latest_feasible_arrival_ ==
              (7 * 60 + 3) - 5);  // TODO use platform change time
  ASSERT_TRUE(&data.feeders_[0].distribution_ ==
              &feeder_distributions.get_distribution(
                  0, 0, distributions_container::arrival));

  ASSERT_TRUE(data.feeders_[1].arrival_time_ == 6 * 60 + 41);
  ASSERT_TRUE(data.feeders_[1].transfer_time_ == 5);
  ASSERT_TRUE(data.feeders_[1].latest_feasible_arrival_ == (7 * 60 + 3) - 5);
  ASSERT_TRUE(&data.feeders_[1].distribution_ ==
              &feeder_distributions.get_distribution(
                  0, 0, distributions_container::arrival));
}

TEST(preceding_arrival_feeders, pd_calc_data_departure) {
  auto schedule = loader::load_schedule(
      "modules/reliability/resources/schedule/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));

  precomputed_distributions_test_container train_distributions({0.1, 0.7, 0.2},
                                                               -1);
  precomputed_distributions_test_container feeder_distributions({0.1, 0.7, 0.2},
                                                                -1);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route node at Darmstadt of train ICE_FR_DA_H
  auto& route_node = *graph_accessor::get_departing_route_edge(
                          *graph_accessor::get_first_route_node(
                              *schedule, schedule1::ICE_FR_DA_H))->_to;
  auto const& light_connection = graph_accessor::get_departing_route_edge(
                                     route_node)->_m._route_edge._conns[0];

  data_departure data(route_node, light_connection, false, *schedule,
                      train_distributions, feeder_distributions,
                      s_t_distributions);

  ASSERT_TRUE(schedule->stations[route_node._station_node->_id]->eva_nr ==
              schedule1::DARMSTADT);
  ASSERT_TRUE(light_connection.d_time == 6 * 60 + 11);
  ASSERT_TRUE(light_connection.a_time == 6 * 60 + 45);

  ASSERT_TRUE(data.scheduled_departure_time_ == light_connection.d_time);
  ASSERT_TRUE(data.largest_delay() == data.maximum_waiting_time_);
  ASSERT_TRUE(!data.is_first_route_node_);

  ASSERT_TRUE(data.train_info_.preceding_arrival_info_.arrival_time_ ==
              6 * 60 + 5);
  ASSERT_TRUE(data.train_info_.preceding_arrival_info_.min_standing_ == 2);
  ASSERT_TRUE(data.train_info_.preceding_arrival_info_.arrival_distribution_ ==
              &train_distributions.get_distribution(
                  0, 0, distributions_container::arrival));

  ASSERT_TRUE(data.maximum_waiting_time_ == 3);
  ASSERT_TRUE(data.feeders_.size() == 2);

  ASSERT_TRUE(data.feeders_[0].arrival_time_ == 5 * 60 + 41);
  ASSERT_TRUE(data.feeders_[0].transfer_time_ == 5);
  ASSERT_TRUE(data.feeders_[0].latest_feasible_arrival_ ==
              (6 * 60 + 11 + 3) - 5);
  ASSERT_TRUE(&data.feeders_[0].distribution_ ==
              &feeder_distributions.get_distribution(
                  0, 0, distributions_container::arrival));

  ASSERT_TRUE(data.feeders_[1].arrival_time_ == 5 * 60 + 56);
  ASSERT_TRUE(data.feeders_[1].transfer_time_ == 5);
  ASSERT_TRUE(data.feeders_[1].latest_feasible_arrival_ ==
              (6 * 60 + 11 + 3) - 5);
  ASSERT_TRUE(&data.feeders_[1].distribution_ ==
              &feeder_distributions.get_distribution(
                  0, 0, distributions_container::arrival));
}

TEST(first_route_node_no_waiting_category, pd_calc_data_departure) {
  auto schedule = loader::load_schedule(
      "modules/reliability/resources/schedule/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));

  distributions_container::precomputed_distributions_container dummy(0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route node at Karlsruhe of train RE_K_S
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule, schedule1::RE_K_S);
  // route edge from Karlsruhe to Stuttgart
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& first_light_conn = first_route_edge->_m._route_edge._conns[0];

  data_departure data(first_route_node, first_light_conn, true, *schedule,
                      dummy, dummy, s_t_distributions);

  ASSERT_TRUE(schedule->stations[first_route_node._station_node->_id]->eva_nr ==
              schedule1::KARLSRUHE);
  ASSERT_TRUE(first_light_conn.d_time == 13 * 60);
  ASSERT_TRUE(first_light_conn.a_time == 13 * 60 + 46);
  ASSERT_TRUE(first_light_conn._full_con->con_info->train_nr == 8);

  ASSERT_TRUE(data.scheduled_departure_time_ == first_light_conn.d_time);
  ASSERT_TRUE(data.largest_delay() == 1);
  ASSERT_TRUE(data.maximum_waiting_time_ == 0);
  ASSERT_TRUE(data.is_first_route_node_);

  auto const& start_distribution =
      data.train_info_.first_departure_distribution_;
  ASSERT_TRUE(equal(start_distribution->sum(), 1.0));
  ASSERT_TRUE(start_distribution->first_minute() == 0);
  ASSERT_TRUE(start_distribution->last_minute() == 1);
  ASSERT_TRUE(equal(start_distribution->probability_equal(0), 0.6));
  ASSERT_TRUE(equal(start_distribution->probability_equal(1), 0.4));

  ASSERT_TRUE(data.feeders_.size() == 0);
}

TEST(check_train_distributions, pd_calc_data_departure) {
  auto schedule = loader::load_schedule(
      "modules/reliability/resources/schedule/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));

  // route node at Darmstadt of train ICE_FR_DA_H
  auto& route_node = *graph_accessor::get_departing_route_edge(
                          *graph_accessor::get_first_route_node(
                              *schedule, schedule1::ICE_FR_DA_H))->_to;
  auto const& light_connection = graph_accessor::get_departing_route_edge(
                                     route_node)->_m._route_edge._conns[0];

  struct train_distributions_test2_container
      : distributions_container::abstract_distributions_container {
    train_distributions_test2_container(unsigned int const route_node_train)
        : route_node_train_(route_node_train) {
      train.init_one_point(0, 1.0);
    }
    probability_distribution const& get_distribution(
        unsigned int const route_node_idx, unsigned int const light_conn_idx,
        distributions_container::event_type const t) const override {
      if (route_node_idx == route_node_train_ && light_conn_idx == 0 &&
          t == distributions_container::arrival)
        return train;
      return fail;
    }

    probability_distribution train;
    probability_distribution fail;
    unsigned int const route_node_train_;
  } train_distributions(route_node._id);

  struct feeder_distributions_test_container
      : distributions_container::abstract_distributions_container {
    feeder_distributions_test_container(unsigned int const route_node_feeder1,
                                        unsigned int const route_node_feeder2)
        : route_node_feeder1_(route_node_feeder1),
          route_node_feeder2_(route_node_feeder2) {
      feeder1.init_one_point(0, 1.0);
      feeder2.init_one_point(0, 1.0);
    }
    probability_distribution const& get_distribution(
        unsigned int const route_node_idx, unsigned int const light_conn_idx,
        distributions_container::event_type const t) const override {
      if (route_node_idx == route_node_feeder1_ && light_conn_idx == 0 &&
          t == distributions_container::arrival)
        return feeder1;
      if (route_node_idx == route_node_feeder2_ && light_conn_idx == 1 &&
          t == distributions_container::arrival)
        return feeder2;
      return fail;
    }

    probability_distribution feeder1;
    probability_distribution feeder2;
    probability_distribution fail;
    unsigned int const route_node_feeder1_;
    unsigned int const route_node_feeder2_;
  } feeder_distributions(/* route node at Darmstadt of train IC_FH_DA */
                         graph_accessor::get_departing_route_edge(
                             *graph_accessor::get_first_route_node(
                                 *schedule, schedule1::IC_FH_DA))->_to->_id,
                         /* route node at Darmstadt of train IC_FH_DA */
                         graph_accessor::get_departing_route_edge(
                             *graph_accessor::get_first_route_node(
                                 *schedule, schedule1::IC_FH_DA))->_to->_id);

  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  data_departure data(route_node, light_connection, false, *schedule,
                      train_distributions, feeder_distributions,
                      s_t_distributions);

  ASSERT_TRUE(data.train_info_.preceding_arrival_info_.arrival_distribution_ ==
              &train_distributions.train);
  ASSERT_TRUE(&data.feeders_[0].distribution_ == &feeder_distributions.feeder1);
  ASSERT_TRUE(&data.feeders_[1].distribution_ == &feeder_distributions.feeder2);
}

TEST(check_start_distribution, pd_calc_data_departure) {
  auto schedule = loader::load_schedule(
      "modules/reliability/resources/schedule/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));

  precomputed_distributions_test_container distributions_container({0.1}, 0);
  struct start_and_travel_test2_distributions : start_and_travel_distributions {
    start_and_travel_test2_distributions() {
      distribution.init_one_point(0, 1.0);
    }
    probability_distribution const& get_start_distribution(
        std::string const& train_category) const override {
      if (train_category == "ICE") return distribution;
      return fail;
    }
    void get_travel_time_distributions(
        std::string const& family, unsigned int const travel_time,
        unsigned int const to_departure_delay,
        std::vector<probability_distribution_cref>& distributions)
        const override {}
    probability_distribution distribution;
    probability_distribution fail;
  } s_t_distributions;

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule, schedule1::ICE_FR_DA_H);
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& first_light_conn = first_route_edge->_m._route_edge._conns[0];

  data_departure data(first_route_node, first_light_conn, true, *schedule,
                      distributions_container, distributions_container,
                      s_t_distributions);

  ASSERT_TRUE(data.train_info_.first_departure_distribution_ ==
              &s_t_distributions.distribution);
}

/* In this test case, largest delay depends on the preceding arrival.
 * All other cases in largest_delay() have been tested
 * in the other test cases */
TEST(check_largest_delay, pd_calc_data_departure) {
  auto schedule = loader::load_schedule(
      "modules/reliability/resources/schedule/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));

  precomputed_distributions_test_container train_distributions(
      {0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1}, -1);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route node at Darmstadt of train ICE_FR_DA_H
  auto& route_node = *graph_accessor::get_departing_route_edge(
                          *graph_accessor::get_first_route_node(
                              *schedule, schedule1::ICE_FR_DA_H))->_to;
  auto const& light_connection = graph_accessor::get_departing_route_edge(
                                     route_node)->_m._route_edge._conns[0];

  data_departure data(route_node, light_connection, false, *schedule,
                      train_distributions, train_distributions,
                      s_t_distributions);

  ASSERT_TRUE(data.maximum_waiting_time_ == 3);
  ASSERT_TRUE(data.largest_delay() == 4);
}
