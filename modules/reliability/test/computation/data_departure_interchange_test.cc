#include "gtest/gtest.h"

#include "motis/loader/loader.h"

#include "motis/core/common/date_time_util.h"

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/computation/data_departure_interchange.h"
#include "motis/reliability/distributions/distributions_container.h"
#include "motis/reliability/graph_accessor.h"

#include "../include/interchange_data_for_tests.h"
#include "../include/start_and_travel_test_distributions.h"
#include "../include/test_container.h"
#include "../include/test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace calc_departure_distribution {

class reliability_data_departure_interchange2 : public test_schedule_setup {
public:
  reliability_data_departure_interchange2()
      : test_schedule_setup("modules/reliability/resources/schedule2/",
                            "20150928") {}
  constexpr static auto KASSEL = "6380201";
  constexpr static auto FRANKFURT = "5744986";
  constexpr static auto STUTTGART = "7309882";
  constexpr static auto ERLANGEN = "0953067";
  constexpr static auto HEILBRONN = "1584227";

  /* train numbers */
  constexpr static unsigned RE_K_F = 1;  // 08:00 --> 10:00
  constexpr static unsigned ICE_F_S = 2;  // 10:10 --> 11:10
  constexpr static unsigned ICE_K_F_S = 3;  // 09:15 --> 10:15, 10:20 --> 11:15
  constexpr static unsigned ICE_S_E = 5;  // 11:32 --> 12:32
  constexpr static unsigned S_H_S = 6;  // 07:15 --> 11:15
  constexpr static unsigned ICE_E_K = 7;  // 12:45 --> 14:15
};

class reliability_data_departure_interchange3 : public test_schedule_setup {
public:
  reliability_data_departure_interchange3()
      : test_schedule_setup("modules/reliability/resources/schedule3/",
                            "20150928") {}
  constexpr static auto FRANKFURT = "1111111";
  constexpr static auto MESSE = "2222222";
  constexpr static auto LANGEN = "3333333";
  constexpr static auto WEST = "4444444";

  constexpr static unsigned ICE_L_H = 1;  // 10:00 --> 10:10
  constexpr static unsigned S_M_W = 2;  // 10:20 --> 10:25
};

TEST_F(reliability_data_departure_interchange2,
       interchange_first_route_node_no_feeders) {
  distributions_container::container dummy;
  distributions_container::container::node dummy_node;
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});
  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);
  interchange_data_for_tests const ic_data(*schedule_, RE_K_F, ICE_F_S, KASSEL,
                                           FRANKFURT, STUTTGART, 8 * 60,
                                           10 * 60, 10 * 60 + 10, 11 * 60 + 10);
  data_departure_interchange data(
      true, ic_data.tail_node_departing_train_,
      *ic_data.arriving_route_edge_.to_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, dummy_arrival_distribution, dummy,
      dummy_node, context(*schedule_, dummy, s_t_distributions));

  ASSERT_TRUE(data.is_first_route_node_);
  ASSERT_TRUE(data.scheduled_departure_time_ ==
              ic_data.departing_light_conn_.d_time_);
  ASSERT_TRUE(data.largest_delay() == 1);
  ASSERT_TRUE(data.maximum_waiting_time_ == 0);
  ASSERT_TRUE(data.feeders_.empty());
  ASSERT_TRUE(data.train_info_.first_departure_distribution_ ==
              &s_t_distributions.start_distribution_);

  ASSERT_TRUE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
              ic_data.arriving_light_conn_.a_time_);
  ASSERT_TRUE(data.interchange_feeder_info_.arrival_distribution_ ==
              &dummy_arrival_distribution);
  ASSERT_TRUE(
      data.interchange_feeder_info_.transfer_time_ ==
      schedule_
          ->stations_[ic_data.tail_node_departing_train_.station_node_->id_]
          ->transfer_time_);
  ASSERT_TRUE(data.interchange_feeder_info_.waiting_time_ == 0);
  ASSERT_TRUE(data.interchange_feeder_info_.latest_feasible_arrival_ ==
              (data.scheduled_departure_time_ +
               data.interchange_feeder_info_.waiting_time_) -
                  data.interchange_feeder_info_.transfer_time_);
}

TEST_F(reliability_data_departure_interchange2,
       interchange_preceding_arrival_no_feeders) {
  distributions_container::test_container train_distributions({0.9, 0.1}, 0);
  distributions_container::container dummy;
  distributions_container::container::node dummy_node;
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route node at Frankfurt of train ICE_K_F_S
  auto& tail_node_departing_train =
      *graph_accessor::get_departing_route_edge(
           *graph_accessor::get_first_route_node(*schedule_, ICE_K_F_S))
           ->to_;
  ASSERT_TRUE(schedule_->stations_[tail_node_departing_train.station_node_->id_]
                  ->eva_nr_ == FRANKFURT);

  // arriving train RE_K_F from Kassel to Frankfurt
  // interchange at Frankfurt (second node of ICE_K_F_S)
  // departing train ICE_K_F_S from Frankfurt to Stuttgart
  interchange_data_for_tests const ic_data(
      *schedule_, RE_K_F, tail_node_departing_train, KASSEL, FRANKFURT,
      STUTTGART, 8 * 60, 10 * 60, 10 * 60 + 20, 11 * 60 + 15);

  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  data_departure_interchange data(
      false, tail_node_departing_train, *ic_data.arriving_route_edge_.to_,
      ic_data.departing_light_conn_, ic_data.arriving_light_conn_,
      dummy_arrival_distribution, train_distributions, dummy_node,
      context(*schedule_, dummy, s_t_distributions));

  // light conn of route edge from Kassel to Frankfurt of train ICE_K_F_S
  auto const& preceding_arrival_light_conn =
      graph_accessor::get_departing_route_edge(
          *graph_accessor::get_first_route_node(*schedule_, ICE_K_F_S))
          ->m_.route_edge_.conns_[0];
  ASSERT_TRUE(preceding_arrival_light_conn.d_time_ ==
              test_util::minutes_to_motis_time(9 * 60 + 15));
  ASSERT_TRUE(preceding_arrival_light_conn.a_time_ ==
              test_util::minutes_to_motis_time(10 * 60 + 15));

  ASSERT_FALSE(data.is_first_route_node_);
  ASSERT_TRUE(data.scheduled_departure_time_ ==
              ic_data.departing_light_conn_.d_time_);
  ASSERT_TRUE(data.largest_delay() == 0);
  ASSERT_TRUE(data.maximum_waiting_time_ == 0);
  ASSERT_TRUE(data.feeders_.empty());
  ASSERT_TRUE(
      data.train_info_.preceding_arrival_info_.scheduled_arrival_time_ ==
      preceding_arrival_light_conn.a_time_);
  ASSERT_TRUE(data.train_info_.preceding_arrival_info_.min_standing_ == 2);
  ASSERT_TRUE(data.train_info_.preceding_arrival_info_.arrival_distribution_ ==
              &train_distributions.dist);

  ASSERT_TRUE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
              ic_data.arriving_light_conn_.a_time_);
  ASSERT_TRUE(data.interchange_feeder_info_.arrival_distribution_ ==
              &dummy_arrival_distribution);
  ASSERT_TRUE(
      data.interchange_feeder_info_.transfer_time_ ==
      schedule_
          ->stations_[ic_data.tail_node_departing_train_.station_node_->id_]
          ->transfer_time_);
  ASSERT_TRUE(data.interchange_feeder_info_.waiting_time_ == 0);
  ASSERT_TRUE(data.interchange_feeder_info_.latest_feasible_arrival_ ==
              (data.scheduled_departure_time_ +
               data.interchange_feeder_info_.waiting_time_) -
                  data.interchange_feeder_info_.transfer_time_);
}

TEST_F(reliability_data_departure_interchange2,
       interchange_first_route_node_feeders_incl_ic) {
  // arriving train ICE_F_S from Frankfurt to Stuttgart
  // interchange at Stuttgart
  // departing train ICE_S_E from Stuttgart to Erlangen
  interchange_data_for_tests const ic_data(
      *schedule_, ICE_F_S, ICE_S_E, FRANKFURT, STUTTGART, ERLANGEN,
      10 * 60 + 10, 11 * 60 + 10, 11 * 60 + 32, 12 * 60 + 32);

  distributions_container::container dummy;
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});
  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  probability_distribution feeder_dist;
  feeder_dist.init({0.9, 0.1}, 0);
  distributions_container::container feeder_distributions;
  auto const& distribution_node = init_feeders_and_get_distribution_node(
      feeder_distributions, ic_data.tail_node_departing_train_,
      ic_data.departing_light_conn_, {0.9, 0.1}, 0, *schedule_);
  ASSERT_EQ(2, distribution_node.predecessors_.size());

  data_departure_interchange data(
      true, ic_data.tail_node_departing_train_,
      *ic_data.arriving_route_edge_.to_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, dummy_arrival_distribution, dummy,
      distribution_node,
      context(*schedule_, feeder_distributions, s_t_distributions));

  ASSERT_TRUE(data.is_first_route_node_);
  ASSERT_EQ(ic_data.departing_light_conn_.d_time_,
            data.scheduled_departure_time_);
  ASSERT_EQ(3, data.largest_delay());
  ASSERT_EQ(1, data.feeders_.size());

  // Feeder ICE_K_F_S
  auto const& feeder = data.feeders_[0];

  ASSERT_EQ(test_util::minutes_to_motis_time(11 * 60 + 15),
            feeder.scheduled_arrival_time_);
  ASSERT_TRUE(feeder.distribution_ == feeder_dist);
  ASSERT_TRUE(
      feeder.transfer_time_ ==
      schedule_
          ->stations_[ic_data.tail_node_departing_train_.station_node_->id_]
          ->transfer_time_);
  ASSERT_TRUE(feeder.latest_feasible_arrival_ ==
              (ic_data.departing_light_conn_.d_time_ - feeder.transfer_time_) +
                  3);

  ASSERT_TRUE(data.maximum_waiting_time_ == 3);
  ASSERT_TRUE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
              ic_data.arriving_light_conn_.a_time_);
  ASSERT_TRUE(data.interchange_feeder_info_.arrival_distribution_ ==
              &dummy_arrival_distribution);
  ASSERT_TRUE(
      data.interchange_feeder_info_.transfer_time_ ==
      schedule_
          ->stations_[ic_data.tail_node_departing_train_.station_node_->id_]
          ->transfer_time_);
  ASSERT_TRUE(data.interchange_feeder_info_.waiting_time_ == 3);
  ASSERT_TRUE(data.interchange_feeder_info_.latest_feasible_arrival_ ==
              (data.scheduled_departure_time_ +
               data.interchange_feeder_info_.waiting_time_) -
                  data.interchange_feeder_info_.transfer_time_);
}

TEST_F(reliability_data_departure_interchange2,
       interchange_first_route_node_feeders_excl_ic) {
  // arriving train S_H_S from Heilbronn to Stuttgart
  // interchange at Stuttgart
  // departing train ICE_S_E from Stuttgart to Erlangen
  interchange_data_for_tests const ic_data(
      *schedule_, S_H_S, ICE_S_E, HEILBRONN, STUTTGART, ERLANGEN, 7 * 60 + 15,
      11 * 60 + 15, 11 * 60 + 32, 12 * 60 + 32);
  distributions_container::container dummy;
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});
  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  probability_distribution feeder_dist;
  feeder_dist.init({0.9, 0.1}, 0);
  distributions_container::container feeder_distributions;
  auto const& distribution_node = init_feeders_and_get_distribution_node(
      feeder_distributions, ic_data.tail_node_departing_train_,
      ic_data.departing_light_conn_, {0.9, 0.1}, 0, *schedule_);

  data_departure_interchange data(
      true, ic_data.tail_node_departing_train_,
      *ic_data.arriving_route_edge_.to_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, dummy_arrival_distribution, dummy,
      distribution_node,
      context(*schedule_, feeder_distributions, s_t_distributions));

  ASSERT_TRUE(data.is_first_route_node_);
  ASSERT_TRUE(data.scheduled_departure_time_ ==
              ic_data.departing_light_conn_.d_time_);
  ASSERT_TRUE(data.largest_delay() == 3);
  ASSERT_TRUE(data.feeders_.size() == 2);

  // Feeder ICE_K_F_S
  {
    auto const& feeder = data.feeders_[0];
    ASSERT_TRUE(feeder.scheduled_arrival_time_ ==
                test_util::minutes_to_motis_time(11 * 60 + 15));
    ASSERT_TRUE(feeder.distribution_ == feeder_dist);
    ASSERT_TRUE(
        feeder.transfer_time_ ==
        schedule_
            ->stations_[ic_data.tail_node_departing_train_.station_node_->id_]
            ->transfer_time_);
    ASSERT_TRUE(
        feeder.latest_feasible_arrival_ ==
        (ic_data.departing_light_conn_.d_time_ - feeder.transfer_time_) + 3);
  }
  // Feeder ICE_F_S
  {
    auto const& feeder = data.feeders_[1];
    ASSERT_TRUE(feeder.scheduled_arrival_time_ ==
                test_util::minutes_to_motis_time(11 * 60 + 10));
    ASSERT_TRUE(feeder.distribution_ == feeder_dist);
    ASSERT_TRUE(
        feeder.transfer_time_ ==
        schedule_
            ->stations_[ic_data.tail_node_departing_train_.station_node_->id_]
            ->transfer_time_);
    ASSERT_TRUE(
        feeder.latest_feasible_arrival_ ==
        (ic_data.departing_light_conn_.d_time_ - feeder.transfer_time_) + 3);
  }

  ASSERT_TRUE(data.maximum_waiting_time_ == 3);
  ASSERT_TRUE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
              ic_data.arriving_light_conn_.a_time_);
  ASSERT_TRUE(data.interchange_feeder_info_.arrival_distribution_ ==
              &dummy_arrival_distribution);
  ASSERT_TRUE(
      data.interchange_feeder_info_.transfer_time_ ==
      schedule_
          ->stations_[ic_data.tail_node_departing_train_.station_node_->id_]
          ->transfer_time_);
  ASSERT_TRUE(data.interchange_feeder_info_.waiting_time_ == 0);
  ASSERT_TRUE(data.interchange_feeder_info_.latest_feasible_arrival_ ==
              (data.scheduled_departure_time_ +
               data.interchange_feeder_info_.waiting_time_) -
                  data.interchange_feeder_info_.transfer_time_);
}

TEST_F(reliability_data_departure_interchange2,
       interchange_first_route_node_no_other_feeder_but_icfeeder) {
  // arriving train ICE_S_E from Stuttgart to Erlangen
  // interchange at Stuttgart
  // departing train ICE_E_K from Erlangen to Kassel
  interchange_data_for_tests const ic_data(
      *schedule_, ICE_S_E, ICE_E_K, STUTTGART, ERLANGEN, KASSEL, 11 * 60 + 32,
      12 * 60 + 32, 12 * 60 + 45, 14 * 60 + 15);
  distributions_container::container dummy;
  distributions_container::container::node dummy_node;
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});
  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  data_departure_interchange data(
      true, ic_data.tail_node_departing_train_,
      *ic_data.arriving_route_edge_.to_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, dummy_arrival_distribution, dummy,
      dummy_node, context(*schedule_, dummy, s_t_distributions));

  ASSERT_TRUE(data.is_first_route_node_);
  ASSERT_TRUE(data.scheduled_departure_time_ ==
              ic_data.departing_light_conn_.d_time_);
  ASSERT_TRUE(data.largest_delay() == 3);
  ASSERT_TRUE(data.feeders_.empty());

  ASSERT_TRUE(data.maximum_waiting_time_ == 3);
  ASSERT_TRUE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
              ic_data.arriving_light_conn_.a_time_);
  ASSERT_TRUE(data.interchange_feeder_info_.arrival_distribution_ ==
              &dummy_arrival_distribution);
  ASSERT_TRUE(
      data.interchange_feeder_info_.transfer_time_ ==
      schedule_
          ->stations_[ic_data.tail_node_departing_train_.station_node_->id_]
          ->transfer_time_);
  ASSERT_TRUE(data.interchange_feeder_info_.waiting_time_ == 3);
  ASSERT_TRUE(data.interchange_feeder_info_.latest_feasible_arrival_ ==
              (data.scheduled_departure_time_ +
               data.interchange_feeder_info_.waiting_time_) -
                  data.interchange_feeder_info_.transfer_time_);
}

TEST_F(reliability_data_departure_interchange3, interchange_walk) {
  // arriving train ICE_L_H from Langen to Frankfurt
  // interchange at Frankfurt and walking to Messe
  // departing train S_M_W from Messe to West
  interchange_data_for_tests const ic_data(
      *schedule_, ICE_L_H, S_M_W, LANGEN, FRANKFURT, MESSE, WEST, 10 * 60,
      10 * 60 + 10, 10 * 60 + 20, 10 * 60 + 25);
  distributions_container::container dummy;
  distributions_container::container::node dummy_node;
  start_and_travel_test_distributions s_t_distributions({0.4, 0.4, 0.2});
  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  data_departure_interchange_walk data(
      true, ic_data.tail_node_departing_train_,
      *ic_data.arriving_route_edge_.to_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, dummy_arrival_distribution, dummy,
      dummy_node, context(*schedule_, dummy, s_t_distributions));

  ASSERT_TRUE(data.is_first_route_node_);
  ASSERT_TRUE(data.scheduled_departure_time_ ==
              ic_data.departing_light_conn_.d_time_);
  ASSERT_TRUE(data.largest_delay() == 2);
  ASSERT_TRUE(data.feeders_.empty());

  ASSERT_TRUE(data.maximum_waiting_time_ == 0);
  ASSERT_TRUE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
              ic_data.arriving_light_conn_.a_time_);
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
