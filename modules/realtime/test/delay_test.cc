#include <iostream>
#include <vector>
#include <algorithm>

#include "catch/catch.hpp"
#include "test_schedule.h"

namespace rt = motis::realtime;

inline motis::time t(int hours, int minutes) { return hours * 60 + minutes; }

constexpr motis::time INV = motis::INVALID_TIME;

TEST_CASE("original events don't have delay", "[delay]") {
  rt::test::test_schedule ts;
  const motis::station* wuerzburg = ts.get_station("Würzburg Hbf");

  for (int offset = 0; offset <= 10; offset++) {
    //    std::cout << "arrival at " << (9+offset) << ":43\n";
    motis::node* route_node;
    motis::light_connection* lc;
    rt::graph_event event(wuerzburg->index, 50, false, t(9 + offset, 43), -1);
    std::tie(route_node, lc) = ts._rts.locate_event(event);
    CHECK(route_node != nullptr);
    CHECK(lc != nullptr);
    if (lc) CHECK(lc->a_time == t(9 + offset, 43));
    CHECK(rt::graph_event(ts._rts.get_schedule_event(event)) == event);
    CHECK(ts._rts._delay_info_manager.get_delay_info(event) == nullptr);
  }

  for (int offset = 0; offset <= 10; offset++) {
    //    std::cout << "departure at " << (9+offset) << ":46\n";
    motis::node* route_node;
    motis::light_connection* lc;
    rt::graph_event event(wuerzburg->index, 50, true, t(9 + offset, 46), -1);
    std::tie(route_node, lc) = ts._rts.locate_event(event);
    CHECK(route_node != nullptr);
    CHECK(lc != nullptr);
    if (lc) CHECK(lc->d_time == t(9 + offset, 46));
    CHECK(rt::graph_event(ts._rts.get_schedule_event(event)) == event);
    CHECK(ts._rts._delay_info_manager.get_delay_info(event) == nullptr);
  }
}

TEST_CASE("simple arrival delay", "[delay]") {
  rt::test::test_schedule ts;
  const motis::station* da_hbf = ts.get_station("Darmstadt Hbf");
  const motis::station* langen = ts.get_station("Langen");
  const motis::station* ffm_hbf = ts.get_station("Frankfurt Hbf");

  // original train: RB 20 - also (13:34, 14:34)
  //                s.a.  s.d.    a.a.  a.d.
  // darmstadt hbf        12:34         12:34
  // langen         12:49 12:51   12:52 12:54
  // frankfurt hbf  13:05         13:08

  std::vector<motis::journey> journeys =
      ts.find_connections(da_hbf, ffm_hbf, t(12, 30));
  REQUIRE(journeys.size() == 1);
  const motis::journey& old_journey = journeys[0];
  ts.check_stops(old_journey, {{da_hbf, t(12, 34), t(12, 34)},
                               {langen, t(12, 49), t(12, 51)},
                               {ffm_hbf, t(13, 05), t(13, 05)}});
  ts.check_transports(old_journey, {{"RB", 20, 1, 3}});

  rt::schedule_event od_da_hbf(da_hbf->index, 20, true, t(12, 34));
  rt::schedule_event oa_langen(langen->index, 20, false, t(12, 49));
  rt::schedule_event od_langen(langen->index, 20, true, t(12, 51));
  rt::schedule_event oa_ffm_hbf(ffm_hbf->index, 20, false, t(13, 5));

  ts._rts._delay_propagator.handle_delay_message(oa_langen, t(12, 52),
                                                 rt::timestamp_reason::IS);
  ts._rts._delay_propagator.process_queue();
  // ts._rts._graph_updater.finish_graph_update();

  ts.check_train(
      {{da_hbf, "", 0, INV, INV, "RB", 20, t(12, 34), t(12, 34)},
       {langen, "RB", 20, t(12, 49), t(12, 52), "RB", 20, t(12, 51), t(12, 54)},
       {ffm_hbf, "RB", 20, t(13, 5), t(13, 8), "", 0, INV, INV}});
}

TEST_CASE("simple departure delay", "[delay]") {
  rt::test::test_schedule ts;
  const motis::station* da_hbf = ts.get_station("Darmstadt Hbf");
  const motis::station* langen = ts.get_station("Langen");
  const motis::station* ffm_hbf = ts.get_station("Frankfurt Hbf");

  // original train: RB 20 - also (13:34, 14:34)
  //                s.a.  s.d.    a.a.  a.d.
  // darmstadt hbf        12:34         12:35
  // langen         12:49 12:51   12:50 12:52
  // frankfurt hbf  13:05         13:06
  rt::schedule_event od_da_hbf(da_hbf->index, 20, true, t(12, 34));
  rt::schedule_event oa_langen(langen->index, 20, false, t(12, 49));
  rt::schedule_event od_langen(langen->index, 20, true, t(12, 51));
  rt::schedule_event oa_ffm_hbf(ffm_hbf->index, 20, false, t(13, 5));

  ts._rts._delay_propagator.handle_delay_message(
      od_da_hbf, t(12, 35), rt::timestamp_reason::FORECAST);
  ts._rts._delay_propagator.process_queue();
  // ts._rts._graph_updater.finish_graph_update();

  ts.check_train(
      {{da_hbf, "", 0, INV, INV, "RB", 20, t(12, 34), t(12, 35)},
       {langen, "RB", 20, t(12, 49), t(12, 50), "RB", 20, t(12, 51), t(12, 52)},
       {ffm_hbf, "RB", 20, t(13, 5), t(13, 6), "", 0, INV, INV}});
}

// SEGFAULTS
TEST_CASE("waiting edge propagation", "[delay]") {
  rt::test::test_schedule ts;
  const motis::station* wuerzburg = ts.get_station("Würzburg Hbf");

  // ICE 51 arrives 12:37, leaves 12:39
  // ICE 55 leaves 12:49

  rt::schedule_event oa_wue(wuerzburg->index, 51, false, t(12, 37));

  ts._rts._delay_propagator.handle_delay_message(
      oa_wue, t(12, 42), rt::timestamp_reason::FORECAST);
  ts._rts._delay_propagator.process_queue();
  // ts._rts._graph_updater.finish_graph_update();
}

TEST_CASE("split route", "[delay]") {
  rt::test::test_schedule ts;
  const motis::station* wuerzburg = ts.get_station("Würzburg Hbf");

  rt::schedule_event oa_wue(wuerzburg->index, 51, false, t(12, 37));

  ts._rts._delay_propagator.handle_delay_message(
      oa_wue, t(13, 45), rt::timestamp_reason::FORECAST);
  ts._rts._delay_propagator.process_queue();
  // ts._rts._graph_updater.finish_graph_update();
}

TEST_CASE("is message back propagation", "[delay]") {
  rt::test::test_schedule ts;
  const motis::station* da_hbf = ts.get_station("Darmstadt Hbf");
  const motis::station* langen = ts.get_station("Langen");
  const motis::station* ffm_hbf = ts.get_station("Frankfurt Hbf");

  // original train: RB 20 - also (13:34, 14:34)
  //                s.a.  s.d.    a.a.  a.d.
  // darmstadt hbf        12:34         12:36
  // langen         12:49 12:51   12:51 12:53
  // frankfurt hbf  13:05         13:07
  rt::schedule_event od_da_hbf(da_hbf->index, 20, true, t(12, 34));
  rt::schedule_event oa_langen(langen->index, 20, false, t(12, 49));
  rt::schedule_event od_langen(langen->index, 20, true, t(12, 51));
  rt::schedule_event oa_ffm_hbf(ffm_hbf->index, 20, false, t(13, 5));

  ts._rts._delay_propagator.handle_delay_message(
      od_da_hbf, t(12, 36), rt::timestamp_reason::FORECAST);
  ts._rts._delay_propagator.process_queue();
  // ts._rts._graph_updater.finish_graph_update();

  ts.check_train(
      {{da_hbf, "", 0, INV, INV, "RB", 20, t(12, 34), t(12, 36)},
       {langen, "RB", 20, t(12, 49), t(12, 51), "RB", 20, t(12, 51), t(12, 53)},
       {ffm_hbf, "RB", 20, t(13, 5), t(13, 7), "", 0, INV, INV}});

  // now we receive an is message and have to fix the timestamps
  // of the preceding events

  ts._rts._delay_propagator.handle_delay_message(od_langen, t(12, 50),
                                                 rt::timestamp_reason::IS);
  ts._rts._delay_propagator.process_queue();
  // ts._rts._graph_updater.finish_graph_update();

  ts.check_train(
      {{da_hbf, "", 0, INV, INV, "RB", 20, t(12, 34), t(12, 36)},
       {langen, "RB", 20, t(12, 49), t(12, 50), "RB", 20, t(12, 51), t(12, 50)},
       {ffm_hbf, "RB", 20, t(13, 5), t(13, 5), "", 0, INV, INV}});
}
