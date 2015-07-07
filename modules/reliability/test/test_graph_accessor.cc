#include "catch/catch.hpp"

#include <iostream>

#include "motis/loader/loader.h"

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/graph_accessor.h"

using namespace motis;
using namespace motis::reliability::graph_accessor;

TEST_CASE("get_previous_light_connection", "[graph_accessor]") {

  auto schedule = load_schedule("../schedule/test");

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

#if 0
// Die Routen scheinen nicht zu stimmen.
TEST_CASE("get_feeders", "[graph_accessor]") {
  auto schedule = loadSchedule("../modules/reliability/resources/schedule/motis");

  auto const first_route_node = schedule->routeIndexToFirstRouteNode[5]; // ICE_FR_DA_H
  auto const first_route_edge = get_departing_route_edge(*first_route_node);

  std::cout << "ok" << std::endl;

  REQUIRE(first_route_node->_station_node->_id == 2);
  REQUIRE(first_route_edge->_to->_station_node->_id == 1);

  std::cout << "ok" << std::endl;

  auto const& first_light_conn = first_route_edge->_m._route_edge._conns[0];

  //REQUIRE(first_light_conn.d_time == 355);
}
#endif
