#include "catch/catch.hpp"

#include <iostream>

#include "motis/loader/loader.h"

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/graph_accessor.h"

using namespace motis;
using namespace motis::reliability::graph_accessor;

TEST_CASE("get_previous_light_connection", "[graph_accessor]") {

  auto schedule = load_text_schedule("../schedule/test");

  auto const first_route_node = schedule->route_index_to_first_route_node[0];
  REQUIRE(first_route_node->_station_node->_id == 5);

  auto const first_route_edge = get_departing_route_edge(*first_route_node);
  auto const& first_light_conn = first_route_edge->_m._route_edge._conns[0];

  REQUIRE(first_light_conn.d_time == 965);

  auto second_route_node = first_route_edge->_to;
  REQUIRE(second_route_node->_station_node->_id == 4);
  REQUIRE(get_arriving_route_edge(*second_route_node) == first_route_edge);

  auto const& second_light_conn =
      get_departing_route_edge(*first_route_node)->_m._route_edge._conns[0];

  auto const& previous_light_conn =
      get_previous_light_connection(*second_route_node, second_light_conn);

  REQUIRE(*previous_light_conn.first == first_light_conn);
  REQUIRE(previous_light_conn.second == 0);
}

TEST_CASE("get_feeders", "[graph_accessor]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node = *schedule->route_index_to_first_route_node[4];
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge = get_departing_route_edge(first_route_node);
  auto const& first_light_conn = first_route_edge->_m._route_edge._conns[0];
  auto const feeders_of_first_route_node =
      get_all_potential_feeders(first_route_node, first_light_conn);

  REQUIRE(first_route_node._station_node->_id == 2);
  REQUIRE(first_route_edge->_to->_station_node->_id == 1);
  REQUIRE(first_light_conn.d_time == 5 * 60 + 55);
  REQUIRE(first_light_conn.a_time == 6 * 60 + 5);
  REQUIRE(feeders_of_first_route_node.size() == 0);

  // route node at Darmstadt
  auto& second_route_node = *first_route_edge->_to;
  // route edge from Darmstadt to Heidelberg
  auto& second_route_edge = *get_departing_route_edge(second_route_node);
  auto const& second_light_conn = second_route_edge._m._route_edge._conns[0];
  auto const all_potential_feeders =
      get_all_potential_feeders(second_route_node, second_light_conn);

  REQUIRE(second_light_conn.d_time == 6 * 60 + 11);
  REQUIRE(all_potential_feeders.size() == 3);

  for (unsigned int i = 0; i < 3; ++i) {
    auto const& feeder_light_conn = *std::get<1>(all_potential_feeders[i]);
    auto const waiting_time = get_waiting_time(
        schedule->waiting_time_rules_, feeder_light_conn, second_light_conn);
    switch (i) {
      case 0: {
        // IC_FH_DA
        REQUIRE(feeder_light_conn._full_con->con_info->train_nr == 3);
        REQUIRE(feeder_light_conn.a_time == 5 * 60 + 41);
        REQUIRE(waiting_time == 3);
        break;
      }
      case 1: {
        // IC_FH_DA
        REQUIRE(feeder_light_conn._full_con->con_info->train_nr == 3);
        REQUIRE(feeder_light_conn.a_time == 5 * 60 + 56);
        REQUIRE(waiting_time == 3);
        break;
      }
      case 2: {
        // RE_MA_DA
        REQUIRE(feeder_light_conn._full_con->con_info->train_nr == 4);
        REQUIRE(feeder_light_conn.a_time == 5 * 60 + 52);
        REQUIRE(waiting_time == 0);
        break;
      }
    }
  }
}

TEST_CASE("get_feeders_first_departure", "[graph_accessor]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");

  // route node at Darmstadt of train IC_DA_H
  auto& first_route_node = *schedule->route_index_to_first_route_node[0];
  // route edge from Darmstadt to Heidelberg
  auto const first_route_edge = get_departing_route_edge(first_route_node);
  // journey 07:00 --> 07:28
  auto const& first_light_conn = first_route_edge->_m._route_edge._conns[1];
  auto const all_potential_feeders =
      get_all_potential_feeders(first_route_node, first_light_conn);

  REQUIRE(first_route_node._station_node->_id == 1);
  REQUIRE(first_route_edge->_to->_station_node->_id == 5);
  REQUIRE(first_light_conn.d_time == 7 * 60);
  REQUIRE(first_light_conn.a_time == 7 * 60 + 28);
  REQUIRE(all_potential_feeders.size() == 3);

  for (unsigned int i = 0; i < 3; ++i) {
    auto const& feeder_light_conn = *std::get<1>(all_potential_feeders[i]);
    auto const waiting_time = get_waiting_time(
        schedule->waiting_time_rules_, feeder_light_conn, first_light_conn);
    switch (i) {
      case 0: {
        // IC_FR_DA
        REQUIRE(feeder_light_conn._full_con->con_info->train_nr == 2);
        REQUIRE(feeder_light_conn.a_time == 6 * 60 + 54);
        REQUIRE(waiting_time == 3);
        break;
      }
      case 1: {
        // IC_FH_DA
        REQUIRE(feeder_light_conn._full_con->con_info->train_nr == 3);
        REQUIRE(feeder_light_conn.a_time == 6 * 60 + 41);
        REQUIRE(waiting_time == 3);
        break;
      }
      case 2: {
        // RE_MA_DA
        REQUIRE(feeder_light_conn._full_con->con_info->train_nr == 4);
        REQUIRE(feeder_light_conn.a_time == 6 * 60 + 52);
        REQUIRE(waiting_time == 0);
        break;
      }
    }
  }
}

TEST_CASE("get_departure_distribution_indices", "[graph_accessor]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");

  // route node at Darmstadt of train IC_DA_H
  auto& first_route_node = *schedule->route_index_to_first_route_node[0];
  // route edge from Darmstadt to Heidelberg
  auto const first_route_edge = get_departing_route_edge(first_route_node);
  // journey 07:00 --> 07:28
  auto const& first_light_conn = first_route_edge->_m._route_edge._conns[1];
  auto const indices =
      get_departure_distribution_indices(*first_route_edge->_to, first_light_conn);

  REQUIRE(indices.first == first_route_node._id);
  REQUIRE(indices.second == 1);
}
