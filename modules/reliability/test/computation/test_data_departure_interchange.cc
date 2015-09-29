#include "catch/catch.hpp"

#include "motis/loader/loader.h"

#include "motis/core/common/date_util.h"

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/distributions_container.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/computation/data_departure_interchange.h"

#include "include/interchange_data_for_tests.h"
#include "include/precomputed_distributions_test_container.h"
#include "include/start_and_travel_test_distributions.h"

using namespace motis;
using namespace motis::reliability;
using namespace motis::reliability::calc_departure_distribution;

namespace schedule2 {
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
}

TEST_CASE("interchange first-route-node no-feeders",
          "[data_departure_interchange]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule2/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));

  distributions_container::precomputed_distributions_container dummy(0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  interchange_data_for_tests const ic_data(
      *schedule, schedule2::RE_K_F, schedule2::ICE_F_S, schedule2::KASSEL,
      schedule2::FRANKFURT, schedule2::STUTTGART, 8 * 60, 10 * 60, 10 * 60 + 10,
      11 * 60 + 10);

  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  data_departure_interchange data(
      true, ic_data.tail_node_departing_train_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, dummy_arrival_distribution, *schedule,
      dummy, s_t_distributions);

  REQUIRE(data.is_first_route_node_);
  REQUIRE(data.scheduled_departure_time_ ==
          ic_data.departing_light_conn_.d_time);
  REQUIRE(data.largest_delay() == 1);
  REQUIRE(data.maximum_waiting_time_ == 0);
  REQUIRE(data.feeders_.size() == 0);
  REQUIRE(data.train_info_.first_departure_distribution_ ==
          &s_t_distributions.start_distribution_);

  REQUIRE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
          ic_data.arriving_light_conn_.a_time);
  REQUIRE(data.interchange_feeder_info_.arrival_distribution_ ==
          &dummy_arrival_distribution);
  REQUIRE(
      data.interchange_feeder_info_.transfer_time_ ==
      schedule->stations[ic_data.tail_node_departing_train_._station_node->_id]
          ->transfer_time);
  REQUIRE(data.interchange_feeder_info_.waiting_time_ == 0);
  REQUIRE(data.interchange_feeder_info_.latest_feasible_arrival_ ==
          (data.scheduled_departure_time_ +
           data.interchange_feeder_info_.waiting_time_) -
              data.interchange_feeder_info_.transfer_time_);
}

TEST_CASE("interchange preceding-arrival no-feeders",
          "[data_departure_interchange]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule2/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));

  precomputed_distributions_test_container precomputed({0.9, 0.1}, 0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route node at Frankfurt of train ICE_K_F_S
  auto& tail_node_departing_train =
      *graph_accessor::get_departing_route_edge(
           *graph_accessor::get_first_route_node(*schedule,
                                                 schedule2::ICE_K_F_S))->_to;
  REQUIRE(schedule->stations[tail_node_departing_train._station_node->_id]
              ->eva_nr == schedule2::FRANKFURT);

  // arriving train RE_K_F from Kassel to Frankfurt
  // interchange at Frankfurt (second node of ICE_K_F_S)
  // departing train ICE_K_F_S from Frankfurt to Stuttgart
  interchange_data_for_tests const ic_data(
      *schedule, schedule2::RE_K_F, tail_node_departing_train,
      schedule2::KASSEL, schedule2::FRANKFURT, schedule2::STUTTGART, 8 * 60,
      10 * 60, 10 * 60 + 20, 11 * 60 + 15);

  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  data_departure_interchange data(
      false, tail_node_departing_train, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, dummy_arrival_distribution, *schedule,
      precomputed, s_t_distributions);

  // light conn of route edge from Kassel to Frankfurt of train ICE_K_F_S
  auto const& preceding_arrival_light_conn =
      graph_accessor::get_departing_route_edge(
          *graph_accessor::get_first_route_node(
              *schedule, schedule2::ICE_K_F_S))->_m._route_edge._conns[0];
  REQUIRE(preceding_arrival_light_conn.d_time == 9 * 60 + 15);
  REQUIRE(preceding_arrival_light_conn.a_time == 10 * 60 + 15);

  REQUIRE_FALSE(data.is_first_route_node_);
  REQUIRE(data.scheduled_departure_time_ ==
          ic_data.departing_light_conn_.d_time);
  REQUIRE(data.largest_delay() == 0);
  REQUIRE(data.maximum_waiting_time_ == 0);
  REQUIRE(data.feeders_.size() == 0);
  REQUIRE(data.train_info_.preceding_arrival_info_.arrival_time_ ==
          preceding_arrival_light_conn.a_time);
  REQUIRE(data.train_info_.preceding_arrival_info_.min_standing_ == 2);
  REQUIRE(data.train_info_.preceding_arrival_info_.arrival_distribution_ ==
          &precomputed.dist);

  REQUIRE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
          ic_data.arriving_light_conn_.a_time);
  REQUIRE(data.interchange_feeder_info_.arrival_distribution_ ==
          &dummy_arrival_distribution);
  REQUIRE(
      data.interchange_feeder_info_.transfer_time_ ==
      schedule->stations[ic_data.tail_node_departing_train_._station_node->_id]
          ->transfer_time);
  REQUIRE(data.interchange_feeder_info_.waiting_time_ == 0);
  REQUIRE(data.interchange_feeder_info_.latest_feasible_arrival_ ==
          (data.scheduled_departure_time_ +
           data.interchange_feeder_info_.waiting_time_) -
              data.interchange_feeder_info_.transfer_time_);
}

TEST_CASE("interchange first-route-node feeders incl. ic",
          "[data_departure_interchange]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule2/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));

  precomputed_distributions_test_container precomputed({0.9, 0.1}, 0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // arriving train ICE_F_S from Frankfurt to Stuttgart
  // interchange at Stuttgart
  // departing train ICE_S_E from Stuttgart to Erlangen
  interchange_data_for_tests const ic_data(
      *schedule, schedule2::ICE_F_S, schedule2::ICE_S_E, schedule2::FRANKFURT,
      schedule2::STUTTGART, schedule2::ERLANGEN, 10 * 60 + 10, 11 * 60 + 10,
      11 * 60 + 32, 12 * 60 + 32);

  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  data_departure_interchange data(
      true, ic_data.tail_node_departing_train_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, dummy_arrival_distribution, *schedule,
      precomputed, s_t_distributions);

  REQUIRE(data.is_first_route_node_);
  REQUIRE(data.scheduled_departure_time_ ==
          ic_data.departing_light_conn_.d_time);
  REQUIRE(data.largest_delay() == 3);
  REQUIRE(data.feeders_.size() == 1);

  // Feeder ICE_K_F_S
  auto const& feeder = data.feeders_[0];

  REQUIRE(feeder.arrival_time_ == 11 * 60 + 15);
  REQUIRE(&feeder.distribution_ == &precomputed.dist);
  REQUIRE(
      feeder.transfer_time_ ==
      schedule->stations[ic_data.tail_node_departing_train_._station_node->_id]
          ->transfer_time);
  REQUIRE(feeder.latest_feasible_arrival_ ==
          (ic_data.departing_light_conn_.d_time - feeder.transfer_time_) + 3);

  REQUIRE(data.maximum_waiting_time_ == 3);
  REQUIRE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
          ic_data.arriving_light_conn_.a_time);
  REQUIRE(data.interchange_feeder_info_.arrival_distribution_ ==
          &dummy_arrival_distribution);
  REQUIRE(
      data.interchange_feeder_info_.transfer_time_ ==
      schedule->stations[ic_data.tail_node_departing_train_._station_node->_id]
          ->transfer_time);
  REQUIRE(data.interchange_feeder_info_.waiting_time_ == 3);
  REQUIRE(data.interchange_feeder_info_.latest_feasible_arrival_ ==
          (data.scheduled_departure_time_ +
           data.interchange_feeder_info_.waiting_time_) -
              data.interchange_feeder_info_.transfer_time_);
}

TEST_CASE("interchange first-route-node feeders excl. ic",
          "[data_departure_interchange]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule2/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));

  precomputed_distributions_test_container precomputed({0.9, 0.1}, 0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // arriving train S_H_S from Heilbronn to Stuttgart
  // interchange at Stuttgart
  // departing train ICE_S_E from Stuttgart to Erlangen
  interchange_data_for_tests const ic_data(
      *schedule, schedule2::S_H_S, schedule2::ICE_S_E, schedule2::HEILBRONN,
      schedule2::STUTTGART, schedule2::ERLANGEN, 7 * 60 + 15, 11 * 60 + 15,
      11 * 60 + 32, 12 * 60 + 32);

  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  data_departure_interchange data(
      true, ic_data.tail_node_departing_train_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, dummy_arrival_distribution, *schedule,
      precomputed, s_t_distributions);

  REQUIRE(data.is_first_route_node_);
  REQUIRE(data.scheduled_departure_time_ ==
          ic_data.departing_light_conn_.d_time);
  REQUIRE(data.largest_delay() == 3);
  REQUIRE(data.feeders_.size() == 2);

  // Feeder ICE_F_S
  {
    auto const& feeder = data.feeders_[0];
    REQUIRE(feeder.arrival_time_ == 11 * 60 + 10);
    REQUIRE(&feeder.distribution_ == &precomputed.dist);
    REQUIRE(feeder.transfer_time_ ==
            schedule->stations[ic_data.tail_node_departing_train_._station_node
                                   ->_id]->transfer_time);
    REQUIRE(feeder.latest_feasible_arrival_ ==
            (ic_data.departing_light_conn_.d_time - feeder.transfer_time_) + 3);
  }
  // Feeder ICE_K_F_S
  {
    auto const& feeder = data.feeders_[1];
    REQUIRE(feeder.arrival_time_ == 11 * 60 + 15);
    REQUIRE(&feeder.distribution_ == &precomputed.dist);
    REQUIRE(feeder.transfer_time_ ==
            schedule->stations[ic_data.tail_node_departing_train_._station_node
                                   ->_id]->transfer_time);
    REQUIRE(feeder.latest_feasible_arrival_ ==
            (ic_data.departing_light_conn_.d_time - feeder.transfer_time_) + 3);
  }

  REQUIRE(data.maximum_waiting_time_ == 3);
  REQUIRE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
          ic_data.arriving_light_conn_.a_time);
  REQUIRE(data.interchange_feeder_info_.arrival_distribution_ ==
          &dummy_arrival_distribution);
  REQUIRE(
      data.interchange_feeder_info_.transfer_time_ ==
      schedule->stations[ic_data.tail_node_departing_train_._station_node->_id]
          ->transfer_time);
  REQUIRE(data.interchange_feeder_info_.waiting_time_ == 0);
  REQUIRE(data.interchange_feeder_info_.latest_feasible_arrival_ ==
          (data.scheduled_departure_time_ +
           data.interchange_feeder_info_.waiting_time_) -
              data.interchange_feeder_info_.transfer_time_);
}

TEST_CASE("interchange first-route-node no other feeder but ic-feeder",
          "[data_departure_interchange]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule2/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));

  precomputed_distributions_test_container precomputed({0.9, 0.1}, 0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // arriving train ICE_S_E from Stuttgart to Erlangen
  // interchange at Stuttgart
  // departing train ICE_E_K from Erlangen to Kassel
  interchange_data_for_tests const ic_data(
      *schedule, schedule2::ICE_S_E, schedule2::ICE_E_K, schedule2::STUTTGART,
      schedule2::ERLANGEN, schedule2::KASSEL, 11 * 60 + 32, 12 * 60 + 32,
      12 * 60 + 45, 14 * 60 + 15);

  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  data_departure_interchange data(
      true, ic_data.tail_node_departing_train_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, dummy_arrival_distribution, *schedule,
      precomputed, s_t_distributions);

  REQUIRE(data.is_first_route_node_);
  REQUIRE(data.scheduled_departure_time_ ==
          ic_data.departing_light_conn_.d_time);
  REQUIRE(data.largest_delay() == 3);
  REQUIRE(data.feeders_.size() == 0);

  REQUIRE(data.maximum_waiting_time_ == 3);
  REQUIRE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
          ic_data.arriving_light_conn_.a_time);
  REQUIRE(data.interchange_feeder_info_.arrival_distribution_ ==
          &dummy_arrival_distribution);
  REQUIRE(
      data.interchange_feeder_info_.transfer_time_ ==
      schedule->stations[ic_data.tail_node_departing_train_._station_node->_id]
          ->transfer_time);
  REQUIRE(data.interchange_feeder_info_.waiting_time_ == 3);
  REQUIRE(data.interchange_feeder_info_.latest_feasible_arrival_ ==
          (data.scheduled_departure_time_ +
           data.interchange_feeder_info_.waiting_time_) -
              data.interchange_feeder_info_.transfer_time_);
}

TEST_CASE("interchange foot", "[data_departure_interchange]") {}
