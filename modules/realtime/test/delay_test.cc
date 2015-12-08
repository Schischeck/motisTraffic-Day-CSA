#include <iostream>
#include <vector>
#include <algorithm>

#include "test_schedule.h"

#include "motis/core/schedule/time.h"

using namespace motis;
namespace rt = motis::realtime;

class realtime_delay_test : public motis::realtime::test::test_schedule {};

inline motis::time t(int h, int m) { return to_motis_time(0, h, m); }

constexpr motis::time INV = motis::INVALID_TIME;

TEST_F(realtime_delay_test, test_unmodified_events) {
  const motis::station* wuerzburg = get_station("Würzburg Hbf");

  for (int offset = 0; offset <= 10; offset++) {
    motis::node* route_node;
    motis::light_connection* lc;
    graph_event event(wuerzburg->index, 50, false, t(9 + offset, 43), -1);
    std::tie(route_node, lc) = _rts.locate_event(event);
    ASSERT_TRUE(route_node != nullptr);
    ASSERT_TRUE(lc != nullptr);
    if (lc) {
      EXPECT_EQ(t(9 + offset, 43), lc->a_time);
    }

    EXPECT_EQ(event, graph_event(_rts.get_schedule_event(event)));
    EXPECT_EQ(nullptr, _rts._delay_info_manager.get_delay_info(event));
  }

  for (int offset = 0; offset <= 10; offset++) {
    motis::node* route_node;
    motis::light_connection* lc;
    graph_event event(wuerzburg->index, 50, true, t(9 + offset, 46), -1);
    std::tie(route_node, lc) = _rts.locate_event(event);
    ASSERT_TRUE(route_node != nullptr);
    ASSERT_TRUE(lc != nullptr);
    if (lc) {
      EXPECT_EQ(t(9 + offset, 46), lc->d_time);
    }
    EXPECT_EQ(event, graph_event(_rts.get_schedule_event(event)));
    EXPECT_EQ(nullptr, _rts._delay_info_manager.get_delay_info(event));
  }
}

TEST_F(realtime_delay_test, test_simple_arrival_delay) {
  const motis::station* da_hbf = get_station("Darmstadt Hbf");
  const motis::station* langen = get_station("Langen");
  const motis::station* ffm_hbf = get_station("Frankfurt Hbf");

  // original train: RB 20 - also (13:34, 14:34)
  //                s.a.  s.d.    a.a.  a.d.
  // darmstadt hbf        12:34         12:34
  // langen         12:49 12:51   12:52 12:54
  // frankfurt hbf  13:05         13:08

  std::vector<motis::journey> journeys =
      find_connections(da_hbf, ffm_hbf, t(12, 30));
  ASSERT_EQ(1, journeys.size());
  const motis::journey& old_journey = journeys[0];
  check_stops(old_journey, {{da_hbf, {INV}, {t(12, 34)}},
                            {langen, {t(12, 49)}, {t(12, 51)}},
                            {ffm_hbf, {t(13, 05)}, {INV}}});
  check_transports(old_journey, {{"RB", 20, 0, 2}});

  schedule_event od_da_hbf(da_hbf->index, 20, true, t(12, 34));
  schedule_event oa_langen(langen->index, 20, false, t(12, 49));
  schedule_event od_langen(langen->index, 20, true, t(12, 51));
  schedule_event oa_ffm_hbf(ffm_hbf->index, 20, false, t(13, 5));

  _rts._delay_propagator.handle_delay_message(oa_langen, t(12, 52),
                                              timestamp_reason::IS);
  _rts._delay_propagator.process_queue();
  // _rts._graph_updater.finish_graph_update();

  check_train(
      {{da_hbf, "", 0, INV, INV, "RB", 20, t(12, 34), t(12, 34)},
       {langen, "RB", 20, t(12, 49), t(12, 52), "RB", 20, t(12, 51), t(12, 54)},
       {ffm_hbf, "RB", 20, t(13, 5), t(13, 8), "", 0, INV, INV}});
}

TEST_F(realtime_delay_test, test_simple_departure_delay) {
  const motis::station* da_hbf = get_station("Darmstadt Hbf");
  const motis::station* langen = get_station("Langen");
  const motis::station* ffm_hbf = get_station("Frankfurt Hbf");

  // original train: RB 20 - also (13:34, 14:34)
  //                s.a.  s.d.    a.a.  a.d.
  // darmstadt hbf        12:34         12:35
  // langen         12:49 12:51   12:50 12:52
  // frankfurt hbf  13:05         13:06
  schedule_event od_da_hbf(da_hbf->index, 20, true, t(12, 34));
  schedule_event oa_langen(langen->index, 20, false, t(12, 49));
  schedule_event od_langen(langen->index, 20, true, t(12, 51));
  schedule_event oa_ffm_hbf(ffm_hbf->index, 20, false, t(13, 5));

  _rts._delay_propagator.handle_delay_message(od_da_hbf, t(12, 35),
                                              timestamp_reason::FORECAST);
  _rts._delay_propagator.process_queue();
  // _rts._graph_updater.finish_graph_update();

  check_train(
      {{da_hbf, "", 0, INV, INV, "RB", 20, t(12, 34), t(12, 35)},
       {langen, "RB", 20, t(12, 49), t(12, 50), "RB", 20, t(12, 51), t(12, 52)},
       {ffm_hbf, "RB", 20, t(13, 5), t(13, 6), "", 0, INV, INV}});
}

TEST_F(realtime_delay_test, test_waiting_edge_propagation) {
  const motis::station* wuerzburg = get_station("Würzburg Hbf");

  // ICE 51 arrives 12:37, leaves 12:39
  // ICE 55 leaves 12:49

  schedule_event oa_wue(wuerzburg->index, 51, false, t(12, 37));

  _rts._delay_propagator.handle_delay_message(oa_wue, t(12, 42),
                                              timestamp_reason::FORECAST);
  _rts._delay_propagator.process_queue();
  // _rts._graph_updater.finish_graph_update();
}

TEST_F(realtime_delay_test, test_split_route) {
  const motis::station* wuerzburg = get_station("Würzburg Hbf");

  schedule_event oa_wue(wuerzburg->index, 51, false, t(12, 37));

  _rts._delay_propagator.handle_delay_message(oa_wue, t(13, 45),
                                              timestamp_reason::FORECAST);
  _rts._delay_propagator.process_queue();
  // _rts._graph_updater.finish_graph_update();
}

TEST_F(realtime_delay_test, test_is_message_back_propagation) {
  const motis::station* da_hbf = get_station("Darmstadt Hbf");
  const motis::station* langen = get_station("Langen");
  const motis::station* ffm_hbf = get_station("Frankfurt Hbf");

  // original train: RB 20 - also (13:34, 14:34)
  //                s.a.  s.d.    a.a.  a.d.
  // darmstadt hbf        12:34         12:36
  // langen         12:49 12:51   12:51 12:53
  // frankfurt hbf  13:05         13:07
  schedule_event od_da_hbf(da_hbf->index, 20, true, t(12, 34));
  schedule_event oa_langen(langen->index, 20, false, t(12, 49));
  schedule_event od_langen(langen->index, 20, true, t(12, 51));
  schedule_event oa_ffm_hbf(ffm_hbf->index, 20, false, t(13, 5));

  _rts._delay_propagator.handle_delay_message(od_da_hbf, t(12, 36),
                                              timestamp_reason::FORECAST);
  _rts._delay_propagator.process_queue();
  // _rts._graph_updater.finish_graph_update();

  check_train(
      {{da_hbf, "", 0, INV, INV, "RB", 20, t(12, 34), t(12, 36)},
       {langen, "RB", 20, t(12, 49), t(12, 51), "RB", 20, t(12, 51), t(12, 53)},
       {ffm_hbf, "RB", 20, t(13, 5), t(13, 7), "", 0, INV, INV}});

  // now we receive an is message and have to fix the timestamps
  // of the preceding events

  _rts._delay_propagator.handle_delay_message(od_langen, t(12, 50),
                                              timestamp_reason::IS);
  _rts._delay_propagator.process_queue();
  // _rts._graph_updater.finish_graph_update();

  check_train(
      {{da_hbf, "", 0, INV, INV, "RB", 20, t(12, 34), t(12, 36)},
       {langen, "RB", 20, t(12, 49), t(12, 50), "RB", 20, t(12, 51), t(12, 50)},
       {ffm_hbf, "RB", 20, t(13, 5), t(13, 5), "", 0, INV, INV}});
}
