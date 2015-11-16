#include <iostream>
#include <vector>
#include <algorithm>
#include <tuple>

#include "test_schedule.h"

namespace rt = motis::realtime;

class realtime_waiting_edges_test
    : public motis::realtime::test::test_schedule {};

inline motis::time t(int hours, int minutes) { return hours * 60 + minutes; }

TEST_F(realtime_waiting_edges_test, test_waiting_edges) {
  const motis::station* wuerzburg = get_station("Würzburg Hbf");

  // find arrival event
  rt::schedule_event event(wuerzburg->index, 51, false, t(10, 37));
  motis::node* route_node;
  motis::light_connection* inc_lc;
  std::tie(route_node, inc_lc) = _rts.locate_event(rt::graph_event(event));

  ASSERT_TRUE(route_node != nullptr);

  const std::vector<rt::single_waiting_edge> edges =
      _rts._waiting_edges.get_edges_from(event, route_node->_route);

  ASSERT_GE(edges.size(), 2);

  EXPECT_TRUE(std::all_of(
      edges.begin(), edges.end(), [&](const rt::single_waiting_edge& e) {
        return static_cast<int>(e._feeder_arrival._station_index) ==
                   wuerzburg->index &&
               static_cast<int>(e._connector_departure._station_index) ==
                   wuerzburg->index;
      }));
  EXPECT_TRUE(std::any_of(
      edges.begin(), edges.end(), [&](const rt::single_waiting_edge& e) {
        return e._connector_departure._schedule_time == t(10, 49) &&
               e._connector_departure._train_nr == 55;
      }));
  EXPECT_TRUE(std::any_of(
      edges.begin(), edges.end(), [&](const rt::single_waiting_edge& e) {
        return e._connector_departure._schedule_time == t(10, 46) &&
               e._connector_departure._train_nr == 50;
      }));
}
