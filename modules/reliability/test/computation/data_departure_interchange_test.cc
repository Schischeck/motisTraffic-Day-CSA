#include "gtest/gtest.h"

#include "motis/loader/loader.h"

#include "motis/core/common/date_util.h"

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/distributions_container.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/computation/data_departure_interchange.h"

#include "../include/interchange_data_for_tests.h"
#include "../include/precomputed_distributions_test_container.h"
#include "../include/start_and_travel_test_distributions.h"
#include "../include/test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace calc_departure_distribution {

class reliability_data_departure_interchange2 : public test_schedule_setup {
public:
  reliability_data_departure_interchange2()
      : test_schedule_setup("modules/reliability/resources/schedule2/",
                            "20150928") {}
  std::string const KASSEL = "6380201";
  std::string const FRANKFURT = "5744986";
  std::string const STUTTGART = "7309882";
  std::string const ERLANGEN = "0953067";
  std::string const HEILBRONN = "1584227";

  /* train numbers */
  short const RE_K_F = 1;  // 08:00 --> 10:00
  short const ICE_F_S = 2;  // 10:10 --> 11:10
  short const ICE_K_F_S = 3;  // 09:15 --> 10:15, 10:20 --> 11:15
  short const ICE_S_E = 5;  // 11:32 --> 12:32
  short const S_H_S = 6;  // 07:15 --> 11:15
  short const ICE_E_K = 7;  // 12:45 --> 14:15
};

class reliability_data_departure_interchange3 : public test_schedule_setup {
public:
  reliability_data_departure_interchange3()
      : test_schedule_setup("modules/reliability/resources/schedule3/",
                            "20150928") {}
  std::string const FRANKFURT = "1111111";
  std::string const MESSE = "2222222";
  std::string const LANGEN = "3333333";
  std::string const WEST = "4444444";

  short const ICE_L_H = 1;  // 10:00 --> 10:10
  short const S_M_W = 2;  // 10:20 --> 10:25
};

TEST_F(reliability_data_departure_interchange2,
       interchange_first_route_node_no_feeders) {
  distributions_container::precomputed_distributions_container dummy(0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  interchange_data_for_tests const ic_data(*schedule_, RE_K_F, ICE_F_S, KASSEL,
                                           FRANKFURT, STUTTGART, 8 * 60,
                                           10 * 60, 10 * 60 + 10, 11 * 60 + 10);

  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  data_departure_interchange data(
      true, ic_data.tail_node_departing_train_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, dummy_arrival_distribution, *schedule_,
      dummy, dummy, s_t_distributions);

  ASSERT_TRUE(data.is_first_route_node_);
  ASSERT_TRUE(data.scheduled_departure_time_ ==
              ic_data.departing_light_conn_.d_time);
  ASSERT_TRUE(data.largest_delay() == 1);
  ASSERT_TRUE(data.maximum_waiting_time_ == 0);
  ASSERT_TRUE(data.feeders_.size() == 0);
  ASSERT_TRUE(data.train_info_.first_departure_distribution_ ==
              &s_t_distributions.start_distribution_);

  ASSERT_TRUE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
              ic_data.arriving_light_conn_.a_time);
  ASSERT_TRUE(data.interchange_feeder_info_.arrival_distribution_ ==
              &dummy_arrival_distribution);
  ASSERT_TRUE(
      data.interchange_feeder_info_.transfer_time_ ==
      schedule_->stations[ic_data.tail_node_departing_train_._station_node->_id]
          ->transfer_time);
  ASSERT_TRUE(data.interchange_feeder_info_.waiting_time_ == 0);
  ASSERT_TRUE(data.interchange_feeder_info_.latest_feasible_arrival_ ==
              (data.scheduled_departure_time_ +
               data.interchange_feeder_info_.waiting_time_) -
                  data.interchange_feeder_info_.transfer_time_);
}

TEST_F(reliability_data_departure_interchange2,
       interchange_preceding_arrival_no_feeders) {
  precomputed_distributions_test_container train_distributions({0.9, 0.1}, 0);
  distributions_container::precomputed_distributions_container dummy(0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route node at Frankfurt of train ICE_K_F_S
  auto& tail_node_departing_train =
      *graph_accessor::get_departing_route_edge(
           *graph_accessor::get_first_route_node(*schedule_, ICE_K_F_S))
           ->_to;
  ASSERT_TRUE(schedule_->stations[tail_node_departing_train._station_node->_id]
                  ->eva_nr == FRANKFURT);

  // arriving train RE_K_F from Kassel to Frankfurt
  // interchange at Frankfurt (second node of ICE_K_F_S)
  // departing train ICE_K_F_S from Frankfurt to Stuttgart
  interchange_data_for_tests const ic_data(
      *schedule_, RE_K_F, tail_node_departing_train, KASSEL, FRANKFURT,
      STUTTGART, 8 * 60, 10 * 60, 10 * 60 + 20, 11 * 60 + 15);

  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  data_departure_interchange data(
      false, tail_node_departing_train, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, dummy_arrival_distribution, *schedule_,
      train_distributions, dummy, s_t_distributions);

  // light conn of route edge from Kassel to Frankfurt of train ICE_K_F_S
  auto const& preceding_arrival_light_conn =
      graph_accessor::get_departing_route_edge(
          *graph_accessor::get_first_route_node(*schedule_, ICE_K_F_S))
          ->_m._route_edge._conns[0];
  ASSERT_TRUE(preceding_arrival_light_conn.d_time ==
              test_util::minutes_to_motis_time(9 * 60 + 15));
  ASSERT_TRUE(preceding_arrival_light_conn.a_time ==
              test_util::minutes_to_motis_time(10 * 60 + 15));

  ASSERT_FALSE(data.is_first_route_node_);
  ASSERT_TRUE(data.scheduled_departure_time_ ==
              ic_data.departing_light_conn_.d_time);
  ASSERT_TRUE(data.largest_delay() == 0);
  ASSERT_TRUE(data.maximum_waiting_time_ == 0);
  ASSERT_TRUE(data.feeders_.size() == 0);
  ASSERT_TRUE(data.train_info_.preceding_arrival_info_.arrival_time_ ==
              preceding_arrival_light_conn.a_time);
  ASSERT_TRUE(data.train_info_.preceding_arrival_info_.min_standing_ == 2);
  ASSERT_TRUE(data.train_info_.preceding_arrival_info_.arrival_distribution_ ==
              &train_distributions.dist);

  ASSERT_TRUE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
              ic_data.arriving_light_conn_.a_time);
  ASSERT_TRUE(data.interchange_feeder_info_.arrival_distribution_ ==
              &dummy_arrival_distribution);
  ASSERT_TRUE(
      data.interchange_feeder_info_.transfer_time_ ==
      schedule_->stations[ic_data.tail_node_departing_train_._station_node->_id]
          ->transfer_time);
  ASSERT_TRUE(data.interchange_feeder_info_.waiting_time_ == 0);
  ASSERT_TRUE(data.interchange_feeder_info_.latest_feasible_arrival_ ==
              (data.scheduled_departure_time_ +
               data.interchange_feeder_info_.waiting_time_) -
                  data.interchange_feeder_info_.transfer_time_);
}

TEST_F(reliability_data_departure_interchange2,
       interchange_first_route_node_feeders_incl_ic) {
  distributions_container::precomputed_distributions_container dummy(0);
  precomputed_distributions_test_container feeder_distributions({0.9, 0.1}, 0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // arriving train ICE_F_S from Frankfurt to Stuttgart
  // interchange at Stuttgart
  // departing train ICE_S_E from Stuttgart to Erlangen
  interchange_data_for_tests const ic_data(
      *schedule_, ICE_F_S, ICE_S_E, FRANKFURT, STUTTGART, ERLANGEN,
      10 * 60 + 10, 11 * 60 + 10, 11 * 60 + 32, 12 * 60 + 32);

  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  data_departure_interchange data(
      true, ic_data.tail_node_departing_train_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, dummy_arrival_distribution, *schedule_,
      dummy, feeder_distributions, s_t_distributions);

  ASSERT_TRUE(data.is_first_route_node_);
  ASSERT_TRUE(data.scheduled_departure_time_ ==
              ic_data.departing_light_conn_.d_time);
  ASSERT_TRUE(data.largest_delay() == 3);
  ASSERT_TRUE(data.feeders_.size() == 1);

  // Feeder ICE_K_F_S
  auto const& feeder = data.feeders_[0];

  ASSERT_TRUE(feeder.arrival_time_ ==
              test_util::minutes_to_motis_time(11 * 60 + 15));
  ASSERT_TRUE(&feeder.distribution_ == &feeder_distributions.dist);
  ASSERT_TRUE(
      feeder.transfer_time_ ==
      schedule_->stations[ic_data.tail_node_departing_train_._station_node->_id]
          ->transfer_time);
  ASSERT_TRUE(feeder.latest_feasible_arrival_ ==
              (ic_data.departing_light_conn_.d_time - feeder.transfer_time_) +
                  3);

  ASSERT_TRUE(data.maximum_waiting_time_ == 3);
  ASSERT_TRUE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
              ic_data.arriving_light_conn_.a_time);
  ASSERT_TRUE(data.interchange_feeder_info_.arrival_distribution_ ==
              &dummy_arrival_distribution);
  ASSERT_TRUE(
      data.interchange_feeder_info_.transfer_time_ ==
      schedule_->stations[ic_data.tail_node_departing_train_._station_node->_id]
          ->transfer_time);
  ASSERT_TRUE(data.interchange_feeder_info_.waiting_time_ == 3);
  ASSERT_TRUE(data.interchange_feeder_info_.latest_feasible_arrival_ ==
              (data.scheduled_departure_time_ +
               data.interchange_feeder_info_.waiting_time_) -
                  data.interchange_feeder_info_.transfer_time_);
}

TEST_F(reliability_data_departure_interchange2,
       interchange_first_route_node_feeders_excl_ic) {
  distributions_container::precomputed_distributions_container dummy(0);
  precomputed_distributions_test_container feeder_distributions({0.9, 0.1}, 0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // arriving train S_H_S from Heilbronn to Stuttgart
  // interchange at Stuttgart
  // departing train ICE_S_E from Stuttgart to Erlangen
  interchange_data_for_tests const ic_data(
      *schedule_, S_H_S, ICE_S_E, HEILBRONN, STUTTGART, ERLANGEN, 7 * 60 + 15,
      11 * 60 + 15, 11 * 60 + 32, 12 * 60 + 32);

  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  data_departure_interchange data(
      true, ic_data.tail_node_departing_train_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, dummy_arrival_distribution, *schedule_,
      dummy, feeder_distributions, s_t_distributions);

  ASSERT_TRUE(data.is_first_route_node_);
  ASSERT_TRUE(data.scheduled_departure_time_ ==
              ic_data.departing_light_conn_.d_time);
  ASSERT_TRUE(data.largest_delay() == 3);
  ASSERT_TRUE(data.feeders_.size() == 2);

  // Feeder ICE_K_F_S
  {
    auto const& feeder = data.feeders_[0];
    ASSERT_TRUE(feeder.arrival_time_ ==
                test_util::minutes_to_motis_time(11 * 60 + 15));
    ASSERT_TRUE(&feeder.distribution_ == &feeder_distributions.dist);
    ASSERT_TRUE(
        feeder.transfer_time_ ==
        schedule_
            ->stations[ic_data.tail_node_departing_train_._station_node->_id]
            ->transfer_time);
    ASSERT_TRUE(feeder.latest_feasible_arrival_ ==
                (ic_data.departing_light_conn_.d_time - feeder.transfer_time_) +
                    3);
  }
  // Feeder ICE_F_S
  {
    auto const& feeder = data.feeders_[1];
    ASSERT_TRUE(feeder.arrival_time_ ==
                test_util::minutes_to_motis_time(11 * 60 + 10));
    ASSERT_TRUE(&feeder.distribution_ == &feeder_distributions.dist);
    ASSERT_TRUE(
        feeder.transfer_time_ ==
        schedule_
            ->stations[ic_data.tail_node_departing_train_._station_node->_id]
            ->transfer_time);
    ASSERT_TRUE(feeder.latest_feasible_arrival_ ==
                (ic_data.departing_light_conn_.d_time - feeder.transfer_time_) +
                    3);
  }

  ASSERT_TRUE(data.maximum_waiting_time_ == 3);
  ASSERT_TRUE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
              ic_data.arriving_light_conn_.a_time);
  ASSERT_TRUE(data.interchange_feeder_info_.arrival_distribution_ ==
              &dummy_arrival_distribution);
  ASSERT_TRUE(
      data.interchange_feeder_info_.transfer_time_ ==
      schedule_->stations[ic_data.tail_node_departing_train_._station_node->_id]
          ->transfer_time);
  ASSERT_TRUE(data.interchange_feeder_info_.waiting_time_ == 0);
  ASSERT_TRUE(data.interchange_feeder_info_.latest_feasible_arrival_ ==
              (data.scheduled_departure_time_ +
               data.interchange_feeder_info_.waiting_time_) -
                  data.interchange_feeder_info_.transfer_time_);
}

TEST_F(reliability_data_departure_interchange2,
       interchange_first_route_node_no_other_feeder_but_icfeeder) {
  distributions_container::precomputed_distributions_container dummy(0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // arriving train ICE_S_E from Stuttgart to Erlangen
  // interchange at Stuttgart
  // departing train ICE_E_K from Erlangen to Kassel
  interchange_data_for_tests const ic_data(
      *schedule_, ICE_S_E, ICE_E_K, STUTTGART, ERLANGEN, KASSEL, 11 * 60 + 32,
      12 * 60 + 32, 12 * 60 + 45, 14 * 60 + 15);

  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  data_departure_interchange data(
      true, ic_data.tail_node_departing_train_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, dummy_arrival_distribution, *schedule_,
      dummy, dummy, s_t_distributions);

  ASSERT_TRUE(data.is_first_route_node_);
  ASSERT_TRUE(data.scheduled_departure_time_ ==
              ic_data.departing_light_conn_.d_time);
  ASSERT_TRUE(data.largest_delay() == 3);
  ASSERT_TRUE(data.feeders_.size() == 0);

  ASSERT_TRUE(data.maximum_waiting_time_ == 3);
  ASSERT_TRUE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
              ic_data.arriving_light_conn_.a_time);
  ASSERT_TRUE(data.interchange_feeder_info_.arrival_distribution_ ==
              &dummy_arrival_distribution);
  ASSERT_TRUE(
      data.interchange_feeder_info_.transfer_time_ ==
      schedule_->stations[ic_data.tail_node_departing_train_._station_node->_id]
          ->transfer_time);
  ASSERT_TRUE(data.interchange_feeder_info_.waiting_time_ == 3);
  ASSERT_TRUE(data.interchange_feeder_info_.latest_feasible_arrival_ ==
              (data.scheduled_departure_time_ +
               data.interchange_feeder_info_.waiting_time_) -
                  data.interchange_feeder_info_.transfer_time_);
}

TEST_F(reliability_data_departure_interchange3, interchange_walk) {
  distributions_container::precomputed_distributions_container dummy(0);
  start_and_travel_test_distributions s_t_distributions({0.4, 0.4, 0.2});

  // arriving train ICE_L_H from Langen to Frankfurt
  // interchange at Frankfurt and walking to Messe
  // departing train S_M_W from Messe to West
  interchange_data_for_tests const ic_data(
      *schedule_, ICE_L_H, S_M_W, LANGEN, FRANKFURT, MESSE, WEST, 10 * 60,
      10 * 60 + 10, 10 * 60 + 20, 10 * 60 + 25);

  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  data_departure_interchange_walk data(
      true, ic_data.tail_node_departing_train_,
      *ic_data.arriving_route_edge_._to->_station_node,
      ic_data.departing_light_conn_, ic_data.arriving_light_conn_,
      dummy_arrival_distribution, *schedule_, dummy, dummy, s_t_distributions);

  ASSERT_TRUE(data.is_first_route_node_);
  ASSERT_TRUE(data.scheduled_departure_time_ ==
              ic_data.departing_light_conn_.d_time);
  ASSERT_TRUE(data.largest_delay() == 2);
  ASSERT_TRUE(data.feeders_.size() == 0);

  ASSERT_TRUE(data.maximum_waiting_time_ == 0);
  ASSERT_TRUE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
              ic_data.arriving_light_conn_.a_time);
  ASSERT_TRUE(data.interchange_feeder_info_.arrival_distribution_ ==
              &dummy_arrival_distribution);
  ASSERT_TRUE(data.interchange_feeder_info_.transfer_time_ == 10);
  ASSERT_TRUE(data.interchange_feeder_info_.waiting_time_ == 0);
  ASSERT_TRUE(data.interchange_feeder_info_.latest_feasible_arrival_ ==
              (data.scheduled_departure_time_ +
               data.interchange_feeder_info_.waiting_time_) -
                  data.interchange_feeder_info_.transfer_time_);
}

}  // namespace calc_departure_distribution
}  // namespace reliability
}  // namespace motis
