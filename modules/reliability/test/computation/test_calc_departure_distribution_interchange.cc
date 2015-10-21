#include "catch/catch.hpp"

#include <iostream>

#include "motis/core/common/date_util.h"

#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

#include "motis/loader/loader.h"

#include "motis/reliability/probability_distribution.h"
#include "motis/reliability/computation/calc_departure_distribution_interchange.h"
#include "motis/reliability/computation/data_departure_interchange.h"

#include "include/interchange_data_for_tests.h"
#include "include/precomputed_distributions_test_container.h"
#include "include/start_and_travel_test_distributions.h"

using namespace motis;
using namespace motis::reliability;

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
short const S_N_E = 4;  // 11:30 --> 15:30
short const ICE_S_E = 5;  // 11:32 --> 12:32
short const S_H_S = 6;  // 07:15 --> 11:15
short const ICE_E_K = 7;  // 12:45 --> 14:15
}

TEST_CASE("ic_feeder_arrived", "[calc_departure_distribution_interchange]") {
  probability_distribution pd;
  pd.init({.1, .1, .1, .1, .1, .1, .1, .1}, 0);  // sum = 0.8

  calc_departure_distribution::data_departure_interchange::
      interchange_feeder_info info;
  info.scheduled_arrival_time_ = 0;
  info.transfer_time_ = 4;
  info.waiting_time_ = 3;
  info.latest_feasible_arrival_ = 5;
  info.arrival_distribution_ = &pd;

  for (unsigned int i = 0; i < 8; i++) {
    probability const prob =
        calc_departure_distribution::interchange::detail::ic_feeder_arrived(
            info, (motis::time)(4 + i));
    REQUIRE(equal(prob, (i + 1) * 0.1));
  }
}

TEST_CASE("ic_feeder_arrives_at_time",
          "[calc_departure_distribution_interchange]") {
  probability_distribution pd;
  pd.init({.1, .1, .1, .1, .1, .1, .1, .1}, 0);  // sum = 0.8

  calc_departure_distribution::data_departure_interchange::
      interchange_feeder_info info;
  info.scheduled_arrival_time_ = 0;
  info.transfer_time_ = 4;
  info.waiting_time_ = 3;
  info.latest_feasible_arrival_ = 5;
  info.arrival_distribution_ = &pd;

  for (unsigned int i = 0; i < 8; i++) {
    probability const prob = calc_departure_distribution::interchange::detail::
        ic_feeder_arrives_at_time(info, (motis::time)(4 + i));
    REQUIRE(equal(prob, 0.1));
  }
}

// first route node without feeders, no waiting, interchange highly reliable
TEST_CASE("compute_departure_distribution_ic1",
          "[calc_departure_distribution_interchange]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule2/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));
  distributions_container::precomputed_distributions_container dummy(0);
  start_and_travel_test_distributions s_t_distributions({.6, .4});
  interchange_data_for_tests const ic_data(
      *schedule, schedule2::RE_K_F, schedule2::ICE_F_S, schedule2::KASSEL,
      schedule2::FRANKFURT, schedule2::STUTTGART, 8 * 60, 10 * 60, 10 * 60 + 10,
      11 * 60 + 10);
  probability_distribution arrival_distribution;
  arrival_distribution.init({.1, .7, .2}, -1);
  calc_departure_distribution::data_departure_interchange data(
      true, ic_data.tail_node_departing_train_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, arrival_distribution, *schedule, dummy,
      dummy, s_t_distributions);
  REQUIRE(data.interchange_feeder_info_.transfer_time_ == 5);
  REQUIRE(data.interchange_feeder_info_.waiting_time_ == 0);

  probability_distribution departure_distribution;
  calc_departure_distribution::interchange::compute_departure_distribution(
      data, departure_distribution);

  REQUIRE(departure_distribution.first_minute() == 0);
  REQUIRE(departure_distribution.last_minute() == 1);
  REQUIRE(equal(departure_distribution.sum(), 1.0));
  REQUIRE(equal(departure_distribution.probability_equal(0), .6));
  REQUIRE(equal(departure_distribution.probability_equal(1), .4));
}

// first route node without feeders, no waiting, interchange unreliable
TEST_CASE("compute_departure_distribution_ic2",
          "[calc_departure_distribution_interchange]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule2/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));
  distributions_container::precomputed_distributions_container dummy(0);
  start_and_travel_test_distributions s_t_distributions({.6, .4});
  interchange_data_for_tests const ic_data(
      *schedule, schedule2::RE_K_F, schedule2::ICE_F_S, schedule2::KASSEL,
      schedule2::FRANKFURT, schedule2::STUTTGART, 8 * 60, 10 * 60, 10 * 60 + 10,
      11 * 60 + 10);
  probability_distribution arrival_distribution;
  arrival_distribution.init({.05, .05, .1, .1, .1, .1, .1, .1, 0.3}, -1);
  calc_departure_distribution::data_departure_interchange data(
      true, ic_data.tail_node_departing_train_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, arrival_distribution, *schedule, dummy,
      dummy, s_t_distributions);
  REQUIRE(data.interchange_feeder_info_.transfer_time_ == 5);
  REQUIRE(data.interchange_feeder_info_.waiting_time_ == 0);

  probability_distribution departure_distribution;
  calc_departure_distribution::interchange::compute_departure_distribution(
      data, departure_distribution);

  REQUIRE(departure_distribution.first_minute() == 0);
  REQUIRE(departure_distribution.last_minute() == 1);
  REQUIRE(
      equal(departure_distribution.probability_equal(0), .6 * .6));  // 10:10
  REQUIRE(
      equal(departure_distribution.probability_equal(1), .4 * .7));  // 10:11
}

// preceding arrival, no other feeders, no waiting for ic feeder
TEST_CASE("compute_departure_distribution_ic3",
          "[calc_departure_distribution_interchange]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule2/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));
  // preceding-arrival: 10:18 - 10:19
  precomputed_distributions_test_container precomputed({0.9, 0.1}, 3);
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
  probability_distribution arrival_distribution;
  arrival_distribution.init({.4, .3, .2, .1}, 14);  // 10:14 - 10:17
  calc_departure_distribution::data_departure_interchange data(
      false, tail_node_departing_train, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, arrival_distribution, *schedule,
      precomputed, precomputed, s_t_distributions);
  REQUIRE(data.interchange_feeder_info_.transfer_time_ == 5);
  REQUIRE(data.interchange_feeder_info_.waiting_time_ == 0);
  REQUIRE(data.train_info_.preceding_arrival_info_.arrival_time_ ==
          10 * 60 + 15);
  REQUIRE(data.train_info_.preceding_arrival_info_.min_standing_ == 2);

  probability_distribution departure_distribution;
  calc_departure_distribution::interchange::compute_departure_distribution(
      data, departure_distribution);

  REQUIRE(departure_distribution.first_minute() == 0);
  REQUIRE(departure_distribution.last_minute() == 1);
  REQUIRE(
      equal(departure_distribution.probability_equal(0), .9 * .7));  // 10:20
  REQUIRE(
      equal(departure_distribution.probability_equal(1), .1 * .9));  // 10:21
}

// first route node, waiting for ic-feeder and other feeder
TEST_CASE("compute_departure_distribution_ic5",
          "[calc_departure_distribution_interchange]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule2/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));
  precomputed_distributions_test_container precomputed(
      {.9, .05, .05}, 12);  // feeder arrival 11:27 - 11:29
  start_and_travel_test_distributions s_t_distributions(
      {.6, .4});  // start distribution 11:32 - 11:33
  // arriving train ICE_F_S from Frankfurt to Stuttgart
  // interchange at Stuttgart
  // departing train ICE_S_E from Stuttgart to Erlangen
  interchange_data_for_tests const ic_data(
      *schedule, schedule2::ICE_F_S, schedule2::ICE_S_E, schedule2::FRANKFURT,
      schedule2::STUTTGART, schedule2::ERLANGEN, 10 * 60 + 10, 11 * 60 + 10,
      11 * 60 + 32, 12 * 60 + 32);
  probability_distribution arrival_distribution;
  arrival_distribution.init({.4, .3, .2, .1}, 16);  // 11:26 - 11:29

  calc_departure_distribution::data_departure_interchange data(
      true, ic_data.tail_node_departing_train_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, arrival_distribution, *schedule,
      precomputed, precomputed, s_t_distributions);

  REQUIRE(data.interchange_feeder_info_.transfer_time_ == 5);
  REQUIRE(data.interchange_feeder_info_.waiting_time_ == 3);
  REQUIRE(data.feeders_.at(0).arrival_time_ == 11 * 60 + 15);
  REQUIRE(data.feeders_.at(0).transfer_time_ == 5);
  REQUIRE(data.feeders_.at(0).latest_feasible_arrival_ == 11 * 60 + 30);

  probability_distribution departure_distribution;
  calc_departure_distribution::interchange::compute_departure_distribution(
      data, departure_distribution);

  REQUIRE(departure_distribution.first_minute() == 0);
  REQUIRE(departure_distribution.last_minute() == 2);
  REQUIRE(equal(departure_distribution.probability_equal(0),
                .6 * .9 * .7));  // 11:32
  probability const prob1 =
      (.4 * .95 * .9) /* train departs with delay at start */
      + (.6 * .05 * .2) /* waits for ic feeder and other feeder */
      + (.6 * .9 * .2) /* waits only for ic feeder */
      + (.6 * .05 * .7) /* waits only for other feeder */;
  REQUIRE(equal(departure_distribution.probability_equal(1), prob1));  // 11:33
  probability const prob2 =
      (.05 * .1) /* waits for ic feeder and other feeder */
      + (.95 * .1) /* waits only for ic feeder */
      + (.05 * .9) /* waits only for other feeder */;
  REQUIRE(equal(departure_distribution.probability_equal(2), prob2));  // 11:34
}
