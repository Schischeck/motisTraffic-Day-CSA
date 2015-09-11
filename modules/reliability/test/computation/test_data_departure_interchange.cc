#include "catch/catch.hpp"

#include "motis/loader/loader.h"

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/distributions_container.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/computation/data_departure_interchange.h"

#include "include/precomputed_distributions_test_container.h"
#include "include/start_and_travel_test_distributions.h"

using namespace motis;
using namespace motis::reliability;
using namespace motis::reliability::calc_departure_distribution;

namespace schedule2 {
short const KASSEL = 1;
short const FRANKFURT = 2;
short const STUTTGART = 3;
short const ERLANGEN = 4;

short const RE_K_F = 0;  // 08:00 --> 10:00
short const ICE_F_S = 1;  // 10:10 --> 11:10
short const S_F_S = 2;  // 07:15 --> 11:15
short const ICE_S_E = 3;  // 11:32 --> 12:32
short const S_S_E = 4;  // 11:30 --> 15:30
short const ICE_E_K = 5;  // 12:45 --> 14:15
short const ICE_K_F_S = 6;  // 09:15 --> 10:15, 10:20 --> 11:15
}

TEST_CASE("interchange first-route-node no-feeders",
          "[data_departure_interchange]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule2/motis");

  distributions_container::precomputed_distributions_container dummy(0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route edge of train RE_K_F from Kassel to Frankfurt
  auto const arriving_route_edge = graph_accessor::get_departing_route_edge(
      *schedule->route_index_to_first_route_node[schedule2::RE_K_F]);
  auto const& arriving_light_conn =
      arriving_route_edge->_m._route_edge._conns[0];

  REQUIRE(arriving_route_edge->_from->_station_node->_id == schedule2::KASSEL);
  REQUIRE(arriving_route_edge->_to->_station_node->_id == schedule2::FRANKFURT);
  REQUIRE(arriving_light_conn.d_time == 8 * 60);
  REQUIRE(arriving_light_conn.a_time == 10 * 60);

  // route node at Frankfurt of train ICE_F_S
  auto& route_node =
      *schedule->route_index_to_first_route_node[schedule2::ICE_F_S];
  // route edge from Frankfurt to Stuttgart
  auto const departing_route_edge =
      graph_accessor::get_departing_route_edge(route_node);
  auto const& departing_light_conn =
      departing_route_edge->_m._route_edge._conns[0];

  REQUIRE(route_node._station_node->_id == schedule2::FRANKFURT);
  REQUIRE(departing_route_edge->_from->_station_node->_id ==
          schedule2::FRANKFURT);
  REQUIRE(departing_route_edge->_to->_station_node->_id ==
          schedule2::STUTTGART);
  REQUIRE(departing_light_conn.d_time == 10 * 60 + 10);
  REQUIRE(departing_light_conn.a_time == 11 * 60 + 10);

  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  data_departure_interchange data(
      true, route_node, departing_light_conn, arriving_light_conn,
      dummy_arrival_distribution, *schedule, dummy, s_t_distributions);

  REQUIRE(data.is_first_route_node_);
  REQUIRE(data.scheduled_departure_time_ == departing_light_conn.d_time);
  REQUIRE(data.largest_delay() == 1);
  REQUIRE(data.maximum_waiting_time_ == 0);
  REQUIRE(data.feeders_.size() == 0);
  REQUIRE(data.train_info_.first_departure_distribution_ ==
          &s_t_distributions.start_distribution_);

  REQUIRE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
          arriving_light_conn.a_time);
  REQUIRE(data.interchange_feeder_info_.arrival_distribution_ ==
          &dummy_arrival_distribution);
  REQUIRE(
      data.interchange_feeder_info_.transfer_time_ ==
      schedule->stations[route_node._station_node->_id]->get_transfer_time());
  REQUIRE(data.interchange_feeder_info_.waiting_time_ == 0);
}

TEST_CASE("interchange preceding-arrival no-feeders",
          "[data_departure_interchange]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule2/motis");

  precomputed_distributions_test_container precomputed({0.9, 0.1}, 0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route edge of train RE_K_F from Kassel to Frankfurt
  auto const arriving_route_edge = graph_accessor::get_departing_route_edge(
      *schedule->route_index_to_first_route_node[schedule2::RE_K_F]);
  auto const& arriving_light_conn =
      arriving_route_edge->_m._route_edge._conns[0];

  REQUIRE(arriving_route_edge->_from->_station_node->_id == schedule2::KASSEL);
  REQUIRE(arriving_route_edge->_to->_station_node->_id == schedule2::FRANKFURT);
  REQUIRE(arriving_light_conn.d_time == 8 * 60);
  REQUIRE(arriving_light_conn.a_time == 10 * 60);

  // light conn of route edge from Kassel to Frankfurt of train ICE_K_F_S
  auto const& preceding_arrival_light_conn =
      graph_accessor::get_departing_route_edge(
          *schedule->route_index_to_first_route_node[schedule2::ICE_K_F_S])
          ->_m._route_edge._conns[0];

  REQUIRE(preceding_arrival_light_conn.d_time == 9 * 60 + 15);
  REQUIRE(preceding_arrival_light_conn.a_time == 10 * 60 + 15);

  // route node at Frankfurt of train ICE_K_F_S
  auto& route_node =
      *graph_accessor::get_departing_route_edge(
           *schedule->route_index_to_first_route_node[schedule2::ICE_K_F_S])
           ->_to;

  REQUIRE(route_node._station_node->_id == schedule2::FRANKFURT);

  // route edge from Frankfurt to Stuttgart
  auto const departing_route_edge =
      graph_accessor::get_departing_route_edge(route_node);
  auto const& departing_light_conn =
      departing_route_edge->_m._route_edge._conns[0];

  REQUIRE(departing_route_edge->_from->_station_node->_id ==
          schedule2::FRANKFURT);
  REQUIRE(departing_route_edge->_to->_station_node->_id ==
          schedule2::STUTTGART);
  REQUIRE(departing_light_conn.d_time == 10 * 60 + 20);
  REQUIRE(departing_light_conn.a_time == 11 * 60 + 15);

  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  data_departure_interchange data(
      false, route_node, departing_light_conn, arriving_light_conn,
      dummy_arrival_distribution, *schedule, precomputed, s_t_distributions);

  REQUIRE_FALSE(data.is_first_route_node_);
  REQUIRE(data.scheduled_departure_time_ == departing_light_conn.d_time);
  REQUIRE(data.largest_delay() == 0);
  REQUIRE(data.maximum_waiting_time_ == 0);
  REQUIRE(data.feeders_.size() == 0);
  REQUIRE(data.train_info_.preceding_arrival_info_.arrival_time_ ==
          preceding_arrival_light_conn.a_time);
  REQUIRE(data.train_info_.preceding_arrival_info_.min_standing_ == 2);
  REQUIRE(data.train_info_.preceding_arrival_info_.arrival_distribution_ ==
          &precomputed.dist);

  REQUIRE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
          arriving_light_conn.a_time);
  REQUIRE(data.interchange_feeder_info_.arrival_distribution_ ==
          &dummy_arrival_distribution);
  REQUIRE(
      data.interchange_feeder_info_.transfer_time_ ==
      schedule->stations[route_node._station_node->_id]->get_transfer_time());
  REQUIRE(data.interchange_feeder_info_.waiting_time_ == 0);
}

TEST_CASE("interchange first-route-node feeders incl. ic",
          "[data_departure_interchange]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule2/motis");

  precomputed_distributions_test_container precomputed({0.9, 0.1}, 0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route edge of train ICE_F_S from Frankfurt to Stuttgart
  auto const& arriving_route_edge =
      *graph_accessor::get_departing_route_edge(
          *schedule->route_index_to_first_route_node[schedule2::ICE_F_S]);
  auto const& arriving_light_conn =
      arriving_route_edge._m._route_edge._conns[0];

  REQUIRE(arriving_route_edge._from->_station_node->_id ==
          schedule2::FRANKFURT);
  REQUIRE(arriving_route_edge._to->_station_node->_id == schedule2::STUTTGART);
  REQUIRE(arriving_light_conn.d_time == 10 * 60 + 10);
  REQUIRE(arriving_light_conn.a_time == 11 * 60 + 10);

  // route node at Stuttgart of train ICE_S_E
  auto const& route_node =
      *schedule->route_index_to_first_route_node[schedule2::ICE_S_E];

  REQUIRE(route_node._station_node->_id == schedule2::STUTTGART);

  // route edge from Stuttgart to Erlangen of train ICE_S_E
  auto const departing_route_edge =
      graph_accessor::get_departing_route_edge(route_node);
  auto const& departing_light_conn =
      departing_route_edge->_m._route_edge._conns[0];

  REQUIRE(departing_route_edge->_from->_station_node->_id ==
          schedule2::STUTTGART);
  REQUIRE(departing_route_edge->_to->_station_node->_id == schedule2::ERLANGEN);
  REQUIRE(departing_light_conn.d_time == 11 * 60 + 32);
  REQUIRE(departing_light_conn.a_time == 12 * 60 + 32);

  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  data_departure_interchange data(
      true, route_node, departing_light_conn, arriving_light_conn,
      dummy_arrival_distribution, *schedule, precomputed, s_t_distributions);

  REQUIRE(data.is_first_route_node_);
  REQUIRE(data.scheduled_departure_time_ == departing_light_conn.d_time);
  REQUIRE(data.largest_delay() == 3);
  REQUIRE(data.feeders_.size() == 1);

  // Feeder ICE_K_F_S
  auto const& feeder = data.feeders_[0];
  auto const& feeder_light_conn =
      graph_accessor::get_departing_route_edge(
          *graph_accessor::get_departing_route_edge(
               *schedule->route_index_to_first_route_node[schedule2::ICE_K_F_S])
               ->_to)->_m._route_edge._conns[0];
  duration const waiting_time = graph_accessor::get_waiting_time(
      schedule->waiting_time_rules_, feeder_light_conn, departing_light_conn);

  REQUIRE(feeder.arrival_time_ == 11 * 60 + 15);
  REQUIRE(&feeder.distribution_ == &precomputed.dist);
  REQUIRE(
      feeder.transfer_time_ ==
      schedule->stations[route_node._station_node->_id]->get_transfer_time());
  REQUIRE(feeder.latest_feasible_arrival_ ==
          (departing_light_conn.d_time - feeder.transfer_time_) + waiting_time);

  REQUIRE(data.maximum_waiting_time_ == 3);
  REQUIRE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
          arriving_light_conn.a_time);
  REQUIRE(data.interchange_feeder_info_.arrival_distribution_ ==
          &dummy_arrival_distribution);
  REQUIRE(
      data.interchange_feeder_info_.transfer_time_ ==
      schedule->stations[route_node._station_node->_id]->get_transfer_time());
  REQUIRE(data.interchange_feeder_info_.waiting_time_ == 3);
}

TEST_CASE("interchange first-route-node feeders excl. ic",
          "[data_departure_interchange]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule2/motis");

  precomputed_distributions_test_container precomputed({0.9, 0.1}, 0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route edge of train S_F_S from Frankfurt to Stuttgart
  auto const& arriving_route_edge =
      *graph_accessor::get_departing_route_edge(
          *schedule->route_index_to_first_route_node[schedule2::S_F_S]);
  auto const& arriving_light_conn =
      arriving_route_edge._m._route_edge._conns[0];

  REQUIRE(arriving_route_edge._from->_station_node->_id ==
          schedule2::FRANKFURT);
  REQUIRE(arriving_route_edge._to->_station_node->_id == schedule2::STUTTGART);
  REQUIRE(arriving_light_conn.d_time == 7 * 60 + 15);
  REQUIRE(arriving_light_conn.a_time == 11 * 60 + 15);

  // route node at Stuttgart of train ICE_S_E
  auto const& route_node =
      *schedule->route_index_to_first_route_node[schedule2::ICE_S_E];

  REQUIRE(route_node._station_node->_id == schedule2::STUTTGART);

  // route edge from Stuttgart to Erlangen of train ICE_S_E
  auto const departing_route_edge =
      graph_accessor::get_departing_route_edge(route_node);
  auto const& departing_light_conn =
      departing_route_edge->_m._route_edge._conns[0];

  REQUIRE(departing_route_edge->_from->_station_node->_id ==
          schedule2::STUTTGART);
  REQUIRE(departing_route_edge->_to->_station_node->_id == schedule2::ERLANGEN);
  REQUIRE(departing_light_conn.d_time == 11 * 60 + 32);
  REQUIRE(departing_light_conn.a_time == 12 * 60 + 32);

  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  data_departure_interchange data(
      true, route_node, departing_light_conn, arriving_light_conn,
      dummy_arrival_distribution, *schedule, precomputed, s_t_distributions);

  REQUIRE(data.is_first_route_node_);
  REQUIRE(data.scheduled_departure_time_ == departing_light_conn.d_time);
  REQUIRE(data.largest_delay() == 3);
  REQUIRE(data.feeders_.size() == 2);

  // Feeder ICE_F_S
  {
    auto const& feeder = data.feeders_[0];
    auto const& feeder_light_conn =
        graph_accessor::get_departing_route_edge(
            *schedule->route_index_to_first_route_node[schedule2::ICE_F_S])
            ->_m._route_edge._conns[0];
    duration const waiting_time = graph_accessor::get_waiting_time(
        schedule->waiting_time_rules_, feeder_light_conn, departing_light_conn);

    REQUIRE(feeder.arrival_time_ == 11 * 60 + 10);
    REQUIRE(&feeder.distribution_ == &precomputed.dist);
    REQUIRE(
        feeder.transfer_time_ ==
        schedule->stations[route_node._station_node->_id]->get_transfer_time());
    REQUIRE(feeder.latest_feasible_arrival_ ==
            (departing_light_conn.d_time - feeder.transfer_time_) +
                waiting_time);
  }
  // Feeder ICE_K_F_S
  {
    auto const& feeder = data.feeders_[1];
    auto const& feeder_light_conn =
        graph_accessor::get_departing_route_edge(
            *graph_accessor::get_departing_route_edge(
                 *schedule
                      ->route_index_to_first_route_node[schedule2::ICE_K_F_S])
                 ->_to)->_m._route_edge._conns[0];
    duration const waiting_time = graph_accessor::get_waiting_time(
        schedule->waiting_time_rules_, feeder_light_conn, departing_light_conn);

    REQUIRE(feeder.arrival_time_ == 11 * 60 + 15);
    REQUIRE(&feeder.distribution_ == &precomputed.dist);
    REQUIRE(
        feeder.transfer_time_ ==
        schedule->stations[route_node._station_node->_id]->get_transfer_time());
    REQUIRE(feeder.latest_feasible_arrival_ ==
            (departing_light_conn.d_time - feeder.transfer_time_) +
                waiting_time);
  }

  REQUIRE(data.maximum_waiting_time_ == 3);
  REQUIRE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
          arriving_light_conn.a_time);
  REQUIRE(data.interchange_feeder_info_.arrival_distribution_ ==
          &dummy_arrival_distribution);
  REQUIRE(
      data.interchange_feeder_info_.transfer_time_ ==
      schedule->stations[route_node._station_node->_id]->get_transfer_time());
  REQUIRE(data.interchange_feeder_info_.waiting_time_ == 0);
}

TEST_CASE("interchange first-route-node no other feeder but ic-feeder",
          "[data_departure_interchange]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule2/motis");

  precomputed_distributions_test_container precomputed({0.9, 0.1}, 0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route edge of train ICE_S_E from Stuttgart to Erlangen
  auto const& arriving_route_edge =
      *graph_accessor::get_departing_route_edge(
          *schedule->route_index_to_first_route_node[schedule2::ICE_S_E]);
  auto const& arriving_light_conn =
      arriving_route_edge._m._route_edge._conns[0];

  REQUIRE(arriving_route_edge._from->_station_node->_id ==
          schedule2::STUTTGART);
  REQUIRE(arriving_route_edge._to->_station_node->_id == schedule2::ERLANGEN);
  REQUIRE(arriving_light_conn.d_time == 11 * 60 + 32);
  REQUIRE(arriving_light_conn.a_time == 12 * 60 + 32);

  // route node at Erlangen of train ICE_E_K
  auto const& route_node =
      *schedule->route_index_to_first_route_node[schedule2::ICE_E_K];

  REQUIRE(route_node._station_node->_id == schedule2::ERLANGEN);

  // route edge from Erlangen to Kassel of train ICE_E_K
  auto const departing_route_edge =
      graph_accessor::get_departing_route_edge(route_node);
  auto const& departing_light_conn =
      departing_route_edge->_m._route_edge._conns[0];

  REQUIRE(departing_route_edge->_from->_station_node->_id ==
          schedule2::ERLANGEN);
  REQUIRE(departing_route_edge->_to->_station_node->_id == schedule2::KASSEL);
  REQUIRE(departing_light_conn.d_time == 12 * 60 + 45);
  REQUIRE(departing_light_conn.a_time == 14 * 60 + 15);

  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  data_departure_interchange data(
      true, route_node, departing_light_conn, arriving_light_conn,
      dummy_arrival_distribution, *schedule, precomputed, s_t_distributions);

  REQUIRE(data.is_first_route_node_);
  REQUIRE(data.scheduled_departure_time_ == departing_light_conn.d_time);
  REQUIRE(data.largest_delay() == 3);
  REQUIRE(data.feeders_.size() == 0);

  REQUIRE(data.maximum_waiting_time_ == 3);
  REQUIRE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
          arriving_light_conn.a_time);
  REQUIRE(data.interchange_feeder_info_.arrival_distribution_ ==
          &dummy_arrival_distribution);
  REQUIRE(
      data.interchange_feeder_info_.transfer_time_ ==
      schedule->stations[route_node._station_node->_id]->get_transfer_time());
  REQUIRE(data.interchange_feeder_info_.waiting_time_ == 3);
}
