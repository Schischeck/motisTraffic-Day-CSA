#if 0

#include <iostream>
#include <vector>
#include <algorithm>
#include <tuple>

#include "catch/catch.hpp"
#include "test_schedule.h"

namespace rt = motis::realtime;

inline motis::time t(int hours, int minutes) { return hours * 60 + minutes; }

TEST_CASE("waiting edges", "[wzr]") {
  rt::test::test_schedule ts;
  const motis::station* wuerzburg = ts.get_station("Würzburg Hbf");

  // find arrival event
  rt::schedule_event event(wuerzburg->index, 51, false, t(10, 37));
  motis::node* route_node;
  motis::light_connection* inc_lc;
  std::tie(route_node, inc_lc) = ts._rts.locate_event(rt::graph_event(event));

  REQUIRE(route_node != nullptr);

  const std::vector<rt::single_waiting_edge> edges =
      ts._rts._waiting_edges.get_edges_from(event, route_node->_route);

  REQUIRE(edges.size() >= 2);

  CHECK(std::all_of(
      edges.begin(), edges.end(), [&](const rt::single_waiting_edge& e) {
        return e._feeder_arrival._station_index == wuerzburg->index &&
               e._connector_departure._station_index == wuerzburg->index;
      }));
  CHECK(std::any_of(
      edges.begin(), edges.end(), [&](const rt::single_waiting_edge& e) {
        return e._connector_departure._schedule_time == t(10, 49) &&
               e._connector_departure._train_nr == 55;
      }));
  CHECK(std::any_of(
      edges.begin(), edges.end(), [&](const rt::single_waiting_edge& e) {
        return e._connector_departure._schedule_time == t(10, 46) &&
               e._connector_departure._train_nr == 50;
      }));
}

#endif
