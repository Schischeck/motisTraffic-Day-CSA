#include <iostream>
#include <vector>
#include <algorithm>
#include <tuple>

#include "motis/core/schedule/time.h"

#include "test_schedule.h"

using namespace motis;
namespace rt = motis::realtime;

class realtime_waiting_edges_test
    : public motis::realtime::test::test_schedule {};

TEST_F(realtime_waiting_edges_test, test_waiting_edges) {
  const motis::station* wuerzburg = get_station("Würzburg Hbf");

  // find arrival event
  schedule_event event(wuerzburg->index, 51, false, to_motis_time(0, 10, 37));
  motis::node* route_node;
  motis::light_connection* inc_lc;
  std::tie(route_node, inc_lc) = _rts.locate_event(graph_event(event));

  ASSERT_TRUE(route_node != nullptr);

  const std::vector<rt::single_waiting_edge> edges =
      _rts._waiting_edges.get_edges_from(event, route_node->_route);

  ASSERT_GE(edges.size(), 2);

  EXPECT_TRUE(std::all_of(
      edges.begin(), edges.end(), [&](const rt::single_waiting_edge& e) {
        return e._feeder_arrival._station_index == wuerzburg->index &&
               e._connector_departure._station_index == wuerzburg->index;
      }));
  EXPECT_TRUE(std::any_of(edges.begin(), edges.end(),
                          [&](const rt::single_waiting_edge& e) {
                            return e._connector_departure._schedule_time ==
                                       to_motis_time(0, 10, 49) &&
                                   e._connector_departure._train_nr == 55;
                          }));
  EXPECT_TRUE(std::any_of(edges.begin(), edges.end(),
                          [&](const rt::single_waiting_edge& e) {
                            return e._connector_departure._schedule_time ==
                                       to_motis_time(0, 10, 46) &&
                                   e._connector_departure._train_nr == 50;
                          }));
}
