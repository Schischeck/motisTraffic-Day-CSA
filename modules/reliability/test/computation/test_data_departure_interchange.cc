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

TEST_CASE("interchange first-route-node no-feeders",
          "[data_departure_interchange]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule2/motis");

  distributions_container::precomputed_distributions_container dummy(0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route edge of train RE_K_F from Kassel to Frankfurt
  auto const arriving_route_edge = graph_accessor::get_departing_route_edge(
      *schedule->route_index_to_first_route_node[1]);
  auto const& arriving_light_conn =
      arriving_route_edge->_m._route_edge._conns[0];

  // route node at Frankfurt of train ICE_F_S
  auto& route_node = *schedule->route_index_to_first_route_node[0];
  // route edge from Frankfurt to Stuttgart
  auto const departing_route_edge =
      graph_accessor::get_departing_route_edge(route_node);
  auto const& departing_light_conn =
      departing_route_edge->_m._route_edge._conns[0];

  REQUIRE(route_node._station_node->_id == 1);
  REQUIRE(arriving_route_edge->_from->_station_node->_id == 2);
  REQUIRE(arriving_route_edge->_to->_station_node->_id == 1);
  REQUIRE(departing_route_edge->_from->_station_node->_id == 1);
  REQUIRE(departing_route_edge->_to->_station_node->_id == 3);
  REQUIRE(arriving_light_conn.d_time == 8 * 60);
  REQUIRE(arriving_light_conn.a_time == 10 * 60);
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

  REQUIRE(data.interchange_feeder_info_.scheduled_arrival_time_ == 10 * 60);
  REQUIRE(data.interchange_feeder_info_.arrival_distribution_ ==
          &dummy_arrival_distribution);
}

TEST_CASE("interchange preceding-arrival no-feeders",
          "[data_departure_interchange]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule2/motis");

  precomputed_distributions_test_container precomputed({0.9, 0.1}, 0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route edge of train RE_K_F from Kassel to Frankfurt
  auto const arriving_route_edge = graph_accessor::get_departing_route_edge(
      *schedule->route_index_to_first_route_node[1]);
  auto const& arriving_light_conn =
      arriving_route_edge->_m._route_edge._conns[0];

  // route node at Frankfurt of train ICE_K_F_S
  auto const& preceding_arrival_light_conn =
      graph_accessor::get_departing_route_edge(
          *schedule->route_index_to_first_route_node[2])
          ->_m._route_edge._conns[0];
  auto& route_node = *graph_accessor::get_departing_route_edge(
                          *schedule->route_index_to_first_route_node[2])->_to;
  // route edge from Frankfurt to Stuttgart
  auto const departing_route_edge =
      graph_accessor::get_departing_route_edge(route_node);
  auto const& departing_light_conn =
      departing_route_edge->_m._route_edge._conns[0];

  REQUIRE(route_node._station_node->_id == 1);
  REQUIRE(arriving_route_edge->_from->_station_node->_id == 2);
  REQUIRE(arriving_route_edge->_to->_station_node->_id == 1);
  REQUIRE(departing_route_edge->_from->_station_node->_id == 1);
  REQUIRE(departing_route_edge->_to->_station_node->_id == 3);
  REQUIRE(arriving_light_conn.d_time == 8 * 60);
  REQUIRE(arriving_light_conn.a_time == 10 * 60);
  REQUIRE(departing_light_conn.d_time == 10 * 60 + 20);
  REQUIRE(departing_light_conn.a_time == 11 * 60 + 15);
  REQUIRE(preceding_arrival_light_conn.d_time == 9 * 60 + 15);
  REQUIRE(preceding_arrival_light_conn.a_time == 10 * 60 + 15);

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

  REQUIRE(data.interchange_feeder_info_.scheduled_arrival_time_ == 10 * 60);
  REQUIRE(data.interchange_feeder_info_.arrival_distribution_ ==
          &dummy_arrival_distribution);
}

TEST_CASE("interchange first-route-node feeders excl ic",
          "[data_departure_interchange]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule2/motis");
}

TEST_CASE("interchange first-route-node feeders incl ic",
          "[data_departure_interchange]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule2/motis");
}
