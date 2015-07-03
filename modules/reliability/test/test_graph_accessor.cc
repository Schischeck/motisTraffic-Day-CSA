#include "catch/catch.hpp"

#include "motis/loader/Loader.h"

#include "motis/core/schedule/Schedule.h"

#include "motis/reliability/graph_accessor.h"

using namespace td;
using namespace motis::reliability::graph_accessor;


TEST_CASE("get_previous_light_connection", "[graph_accessor]") {

  auto schedule = td::loadSchedule("../schedule/test");

  auto first_route_node = schedule->routeIndexToFirstRouteNode[0];
  REQUIRE(first_route_node->_stationNode->_id == 5);

  auto first_route_edge = get_departing_route_edge(*first_route_node);

  REQUIRE(first_route_edge->_m._routeEdge._conns[0].dTime == 965);

  auto second_route_node = first_route_edge->_to;
  REQUIRE(second_route_node->_stationNode->_id == 4);

  REQUIRE(get_arriving_route_edge(*second_route_node) == first_route_edge);
}
