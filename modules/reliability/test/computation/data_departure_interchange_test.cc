#include "gtest/gtest.h"

#include "motis/loader/loader.h"

#include "motis/core/common/date_time_util.h"

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/computation/data_departure_interchange.h"
#include "motis/reliability/distributions/distributions_container.h"
#include "motis/reliability/graph_accessor.h"

#include "../include/interchange_data_for_tests.h"
#include "../include/schedules/schedule2.h"
#include "../include/schedules/schedule3.h"
#include "../include/start_and_travel_test_distributions.h"
#include "../include/test_container.h"
#include "../include/test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace calc_departure_distribution {

class reliability_data_departure_interchange2 : public test_schedule_setup {
public:
  reliability_data_departure_interchange2()
      : test_schedule_setup(schedule2::PATH, schedule2::DATE) {}
};

class reliability_data_departure_interchange3 : public test_schedule_setup {
public:
  reliability_data_departure_interchange3()
      : test_schedule_setup(schedule3::PATH, schedule3::DATE) {}
};

TEST_F(reliability_data_departure_interchange2,
       interchange_first_route_node_no_feeders) {
  distributions_container::container dummy;
  distributions_container::container::node dummy_node;
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});
  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);
  interchange_data_for_tests const ic_data(
      *schedule_, schedule2::RE_K_F, schedule2::ICE_F_S, schedule2::KASSEL.eva_,
      schedule2::FRANKFURT.eva_, schedule2::STUTTGART.eva_, 8 * 60, 10 * 60,
      10 * 60 + 10, 11 * 60 + 10);
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
  auto& tail_node_departing_train = *graph_accessor::get_departing_route_edge(
                                         *graph_accessor::get_first_route_node(
                                             *schedule_, schedule2::ICE_K_F_S))
                                         ->to_;
  ASSERT_TRUE(schedule_->stations_[tail_node_departing_train.station_node_->id_]
                  ->eva_nr_ == schedule2::FRANKFURT.eva_);

  // arriving train RE_K_F from Kassel to Frankfurt
  // interchange at Frankfurt (second node of ICE_K_F_S)
  // departing train ICE_K_F_S from Frankfurt to Stuttgart
  interchange_data_for_tests const ic_data(
      *schedule_, schedule2::RE_K_F, tail_node_departing_train,
      schedule2::KASSEL.eva_, schedule2::FRANKFURT.eva_,
      schedule2::STUTTGART.eva_, 8 * 60, 10 * 60, 10 * 60 + 20, 11 * 60 + 15);

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
          *graph_accessor::get_first_route_node(*schedule_,
                                                schedule2::ICE_K_F_S))
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
              &train_distributions.dist_);

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
      *schedule_, schedule2::ICE_F_S, schedule2::ICE_S_E,
      schedule2::FRANKFURT.eva_, schedule2::STUTTGART.eva_,
      schedule2::ERLANGEN.eva_, 10 * 60 + 10, 11 * 60 + 10, 11 * 60 + 32,
      12 * 60 + 32);

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
      *schedule_, schedule2::S_H_S, schedule2::ICE_S_E,
      schedule2::HEILBRONN.eva_, schedule2::STUTTGART.eva_,
      schedule2::ERLANGEN.eva_, 7 * 60 + 15, 11 * 60 + 15, 11 * 60 + 32,
      12 * 60 + 32);
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
      *schedule_, schedule2::ICE_S_E, schedule2::ICE_E_K,
      schedule2::STUTTGART.eva_, schedule2::ERLANGEN.eva_,
      schedule2::KASSEL.eva_, 11 * 60 + 32, 12 * 60 + 32, 12 * 60 + 45,
      14 * 60 + 15);
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
      *schedule_, schedule3::ICE_L_H, schedule3::S_M_W, schedule3::LANGEN.eva_,
      schedule3::FRANKFURT.eva_, schedule3::MESSE.eva_, schedule3::WEST.eva_,
      10 * 60, 10 * 60 + 10, 10 * 60 + 20, 10 * 60 + 25);
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
