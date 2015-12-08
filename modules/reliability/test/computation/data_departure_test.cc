#include "gtest/gtest.h"

#include "motis/loader/loader.h"

#include "motis/core/common/date_util.h"

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/computation/data_arrival.h"
#include "motis/reliability/computation/data_departure.h"
#include "motis/reliability/distributions/distributions_container.h"
#include "motis/reliability/graph_accessor.h"

#include "../include/start_and_travel_test_distributions.h"
#include "../include/test_container.h"
#include "../include/test_schedule_setup.h"
#include "../include/test_util.h"

namespace motis {
namespace reliability {
namespace calc_departure_distribution {

class reliability_data_departure : public test_schedule_setup {
public:
  reliability_data_departure()
      : test_schedule_setup("modules/reliability/resources/schedule/",
                            "20150928") {}
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
};

TEST_F(reliability_data_departure, first_route_node_no_feeders) {
  distributions_container::container dummy;
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule_, ICE_FR_DA_H);
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& first_light_conn = first_route_edge->_m._route_edge._conns[0];

  data_departure data(first_route_node, first_light_conn, true, dummy,
                      context(*schedule_, dummy, s_t_distributions));

  ASSERT_TRUE(
      schedule_->stations[first_route_node._station_node->_id]->eva_nr ==
      FRANKFURT);
  ASSERT_TRUE(first_light_conn.d_time ==
              test_util::minutes_to_motis_time(5 * 60 + 55));
  ASSERT_TRUE(first_light_conn.a_time ==
              test_util::minutes_to_motis_time(6 * 60 + 5));
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

TEST_F(reliability_data_departure, preceding_arrival_no_feeders) {
  distributions_container::test_container train_distributions({0.1, 0.7, 0.2},
                                                              -1);
  distributions_container::container dummy;
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route node at Hanau of train ICE_HA_W_HE
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule_, ICE_HA_W_HE);
  // route node at Wuerzburg
  auto second_route_node =
      graph_accessor::get_departing_route_edge(first_route_node)->_to;
  // route edge from Wuerzburg to Heilbronn
  auto const route_edge =
      graph_accessor::get_departing_route_edge(*second_route_node);
  auto const& light_connection = route_edge->_m._route_edge._conns[0];

  data_departure data(*second_route_node, light_connection, false,
                      train_distributions,
                      context(*schedule_, dummy, s_t_distributions));

  ASSERT_TRUE(
      schedule_->stations[second_route_node->_station_node->_id]->eva_nr ==
      WURZBURG);
  ASSERT_TRUE(light_connection.d_time ==
              test_util::minutes_to_motis_time(10 * 60 + 34));
  ASSERT_TRUE(light_connection.a_time ==
              test_util::minutes_to_motis_time(11 * 60 + 7));

  ASSERT_TRUE(data.scheduled_departure_time_ == light_connection.d_time);
  ASSERT_TRUE(data.largest_delay() == 1);
  ASSERT_TRUE(!data.is_first_route_node_);
  ASSERT_TRUE(data.maximum_waiting_time_ == 0);
  ASSERT_TRUE(data.feeders_.size() == 0);

  ASSERT_TRUE(
      data.train_info_.preceding_arrival_info_.scheduled_arrival_time_ ==
      test_util::minutes_to_motis_time(10 * 60 + 32));
  ASSERT_TRUE(data.train_info_.preceding_arrival_info_.min_standing_ == 2);
  ASSERT_TRUE(data.train_info_.preceding_arrival_info_.arrival_distribution_ ==
              &train_distributions.get_distribution(
                  distributions_container::container::key()));
}

TEST_F(reliability_data_departure, first_route_node_feeders) {
  distributions_container::container dummy;
  distributions_container::test_container feeder_distributions({0.1, 0.7, 0.2},
                                                               -1);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route node at Darmstadt of train IC_DA_H
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule_, IC_DA_H);
  // route edge from Darmstadt to Heidelberg
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  // light connection d07:00 a07:28
  auto const& light_connection = first_route_edge->_m._route_edge._conns[1];

  data_departure data(
      first_route_node, light_connection, true, dummy,
      context(*schedule_, feeder_distributions, s_t_distributions));

  ASSERT_TRUE(
      schedule_->stations[first_route_node._station_node->_id]->eva_nr ==
      DARMSTADT);
  ASSERT_TRUE(light_connection.d_time ==
              test_util::minutes_to_motis_time(7 * 60));
  ASSERT_TRUE(light_connection.a_time ==
              test_util::minutes_to_motis_time(7 * 60 + 28));

  ASSERT_TRUE(data.scheduled_departure_time_ == light_connection.d_time);
  ASSERT_TRUE(data.largest_delay() == data.maximum_waiting_time_);
  ASSERT_TRUE(data.is_first_route_node_);

  ASSERT_TRUE(data.train_info_.first_departure_distribution_ ==
              &s_t_distributions.get_start_distribution("dummy"));

  ASSERT_TRUE(data.maximum_waiting_time_ == 3);
  ASSERT_TRUE(data.feeders_.size() == 2);

  {
    auto const& feeder = data.feeders_[0];
    ASSERT_TRUE(feeder.scheduled_arrival_time_ ==
                test_util::minutes_to_motis_time(6 * 60 + 41));
    ASSERT_TRUE(feeder.transfer_time_ == 5);
    ASSERT_TRUE(feeder.latest_feasible_arrival_ ==
                test_util::minutes_to_motis_time((7 * 60 + 3) - 5));
    ASSERT_TRUE(&feeder.distribution_ ==
                &feeder_distributions.get_distribution(
                    distributions_container::container::key()));
  }
  {
    auto const& feeder = data.feeders_[1];
    ASSERT_TRUE(feeder.scheduled_arrival_time_ ==
                test_util::minutes_to_motis_time(6 * 60 + 54));
    ASSERT_TRUE(feeder.transfer_time_ == 5);  // TODO use platform change time
    ASSERT_TRUE(feeder.latest_feasible_arrival_ ==
                test_util::minutes_to_motis_time(
                    (7 * 60 + 3) - 5));  // TODO use platform change time
    ASSERT_TRUE(&feeder.distribution_ ==
                &feeder_distributions.get_distribution(
                    distributions_container::container::key()));
  }
}

TEST_F(reliability_data_departure, preceding_arrival_feeders) {
  distributions_container::test_container train_distributions({0.1, 0.7, 0.2},
                                                              -1);
  distributions_container::test_container feeder_distributions({0.1, 0.7, 0.2},
                                                               -1);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route node at Darmstadt of train ICE_FR_DA_H
  auto& route_node =
      *graph_accessor::get_departing_route_edge(
           *graph_accessor::get_first_route_node(*schedule_, ICE_FR_DA_H))
           ->_to;
  auto const& light_connection =
      graph_accessor::get_departing_route_edge(route_node)
          ->_m._route_edge._conns[0];

  data_departure data(
      route_node, light_connection, false, train_distributions,
      context(*schedule_, feeder_distributions, s_t_distributions));

  ASSERT_TRUE(schedule_->stations[route_node._station_node->_id]->eva_nr ==
              DARMSTADT);
  ASSERT_TRUE(light_connection.d_time ==
              test_util::minutes_to_motis_time(6 * 60 + 11));
  ASSERT_TRUE(light_connection.a_time ==
              test_util::minutes_to_motis_time(6 * 60 + 45));

  ASSERT_TRUE(data.scheduled_departure_time_ == light_connection.d_time);
  ASSERT_TRUE(data.largest_delay() == data.maximum_waiting_time_);
  ASSERT_TRUE(!data.is_first_route_node_);

  ASSERT_TRUE(
      data.train_info_.preceding_arrival_info_.scheduled_arrival_time_ ==
      test_util::minutes_to_motis_time(6 * 60 + 5));
  ASSERT_TRUE(data.train_info_.preceding_arrival_info_.min_standing_ == 2);
  ASSERT_TRUE(data.train_info_.preceding_arrival_info_.arrival_distribution_ ==
              &train_distributions.get_distribution(
                  distributions_container::container::key()));

  ASSERT_TRUE(data.maximum_waiting_time_ == 3);
  ASSERT_TRUE(data.feeders_.size() == 2);

  ASSERT_TRUE(data.feeders_[0].scheduled_arrival_time_ ==
              test_util::minutes_to_motis_time(5 * 60 + 41));
  ASSERT_TRUE(data.feeders_[0].transfer_time_ == 5);
  ASSERT_TRUE(data.feeders_[0].latest_feasible_arrival_ ==
              test_util::minutes_to_motis_time((6 * 60 + 11 + 3) - 5));
  ASSERT_TRUE(&data.feeders_[0].distribution_ ==
              &feeder_distributions.get_distribution(
                  distributions_container::container::key()));

  ASSERT_TRUE(data.feeders_[1].scheduled_arrival_time_ ==
              test_util::minutes_to_motis_time(5 * 60 + 56));
  ASSERT_TRUE(data.feeders_[1].transfer_time_ == 5);
  ASSERT_TRUE(data.feeders_[1].latest_feasible_arrival_ ==
              test_util::minutes_to_motis_time((6 * 60 + 11 + 3) - 5));
  ASSERT_TRUE(&data.feeders_[1].distribution_ ==
              &feeder_distributions.get_distribution(
                  distributions_container::container::key()));
}

TEST_F(reliability_data_departure, first_route_node_no_waiting_category) {
  distributions_container::container dummy;
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route node at Karlsruhe of train RE_K_S
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule_, RE_K_S);
  // route edge from Karlsruhe to Stuttgart
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& first_light_conn = first_route_edge->_m._route_edge._conns[0];

  data_departure data(first_route_node, first_light_conn, true, dummy,
                      context(*schedule_, dummy, s_t_distributions));

  ASSERT_TRUE(
      schedule_->stations[first_route_node._station_node->_id]->eva_nr ==
      KARLSRUHE);
  ASSERT_TRUE(first_light_conn.d_time ==
              test_util::minutes_to_motis_time(13 * 60));
  ASSERT_TRUE(first_light_conn.a_time ==
              test_util::minutes_to_motis_time(13 * 60 + 46));
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

bool key_equals(distributions_container::container::key const& a,
                distributions_container::container::key const& b) {
  return a.category_ == b.category_ &&
         a.line_identifier_ == b.line_identifier_ &&
         a.scheduled_event_time_ == b.scheduled_event_time_ &&
         a.station_index_ == b.station_index_ && a.train_id_ == b.train_id_ &&
         a.type_ == b.type_;
}

TEST_F(reliability_data_departure, check_train_distributions) {
  // route node at Darmstadt of train ICE_FR_DA_H
  auto& route_node =
      *graph_accessor::get_departing_route_edge(
           *graph_accessor::get_first_route_node(*schedule_, ICE_FR_DA_H))
           ->_to;

  using namespace distributions_container;

  struct train_distributions_test_container : container {
    train_distributions_test_container(container::key const& k) : key_(k) {
      train_.init_one_point(0, 1.0);
    }
    probability_distribution const& get_distribution(
        container::key const& k) const override {
      if (key_equals(k, key_)) {
        return train_;
      }
      return fail_;
    }
    probability_distribution train_, fail_;
    distributions_container::container::key const key_;
  } train_distributions(to_container_key(
      route_node, graph_accessor::get_arriving_route_edge(route_node)
                      ->_m._route_edge._conns[0],
      time_util::arrival, *schedule_));

  /* route node at Darmstadt of train IC_FH_DA */
  auto const& last_route_node_IC_FH_DA =
      *graph_accessor::get_departing_route_edge(
           *graph_accessor::get_first_route_node(*schedule_, IC_FH_DA))
           ->_to;
  auto const& route_edge_IC_FH_DA =
      *graph_accessor::get_arriving_route_edge(last_route_node_IC_FH_DA);

  struct feeder_distributions_test_container
      : distributions_container::container {
    feeder_distributions_test_container(container::key const& key_feeder1,
                                        container::key const& key_feeder2)
        : key_feeder1_(key_feeder1), key_feeder2_(key_feeder2) {
      feeder1_.init_one_point(0, 1.0);
      feeder2_.init_one_point(0, 1.0);
    }
    probability_distribution const& get_distribution(
        container::key const& k) const override {
      if (key_equals(k, key_feeder1_)) {
        return feeder1_;
      } else if (key_equals(k, key_feeder2_)) {
        return feeder2_;
      }
      return fail_;
    }
    probability_distribution feeder1_, feeder2_, fail_;
    container::key key_feeder1_, key_feeder2_;
  } feeder_distributions(/* IC_FH_DA arrival 05:41 */
                         to_container_key(
                             last_route_node_IC_FH_DA,
                             route_edge_IC_FH_DA._m._route_edge._conns[0],

                             time_util::arrival, *schedule_),
                         /* IC_FH_DA arrival 05:56 */
                         to_container_key(
                             last_route_node_IC_FH_DA,
                             route_edge_IC_FH_DA._m._route_edge._conns[1],
                             time_util::arrival, *schedule_));

  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});
  auto const& light_connection =
      graph_accessor::get_departing_route_edge(route_node)
          ->_m._route_edge._conns[0];

  data_departure data(
      route_node, light_connection, false, train_distributions,
      context(*schedule_, feeder_distributions, s_t_distributions));

  ASSERT_TRUE(data.train_info_.preceding_arrival_info_.arrival_distribution_ ==
              &train_distributions.train_);
  ASSERT_TRUE(&data.feeders_[0].distribution_ ==
              &feeder_distributions.feeder1_);
  ASSERT_TRUE(&data.feeders_[1].distribution_ ==
              &feeder_distributions.feeder2_);
}

TEST_F(reliability_data_departure, check_start_distribution) {
  distributions_container::test_container distributions_container({0.1}, 0);
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
        std::string const&, unsigned int const, unsigned int const,
        std::vector<probability_distribution_cref>&) const override {}
    probability_distribution distribution;
    probability_distribution fail;
  } s_t_distributions;

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule_, ICE_FR_DA_H);
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& first_light_conn = first_route_edge->_m._route_edge._conns[0];

  data_departure data(
      first_route_node, first_light_conn, true, distributions_container,
      context(*schedule_, distributions_container, s_t_distributions));

  ASSERT_TRUE(data.train_info_.first_departure_distribution_ ==
              &s_t_distributions.distribution);
}

/* In this test case, largest delay depends on the preceding arrival.
 * All other cases in largest_delay() have been tested
 * in the other test cases */
TEST_F(reliability_data_departure, check_largest_delay) {
  distributions_container::test_container train_distributions(
      {0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1}, -1);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route node at Darmstadt of train ICE_FR_DA_H
  auto& route_node =
      *graph_accessor::get_departing_route_edge(
           *graph_accessor::get_first_route_node(*schedule_, ICE_FR_DA_H))
           ->_to;
  auto const& light_connection =
      graph_accessor::get_departing_route_edge(route_node)
          ->_m._route_edge._conns[0];

  data_departure data(
      route_node, light_connection, false, train_distributions,
      context(*schedule_, train_distributions, s_t_distributions));

  ASSERT_TRUE(data.maximum_waiting_time_ == 3);
  ASSERT_TRUE(data.largest_delay() == 4);
}

}  // namespace calc_departure_distribution
}  // namespace reliability
}  // namespace motis
