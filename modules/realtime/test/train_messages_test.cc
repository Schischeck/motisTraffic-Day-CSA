#include <iostream>
#include <vector>
#include <algorithm>

#include "catch/catch.hpp"
#include "test_schedule.h"

namespace rt = motis::realtime;

inline motis::time t(int hours, int minutes) { return hours * 60 + minutes; }

constexpr motis::time INV = motis::INVALID_TIME;

TEST_CASE("additional train with existing category", "[msgs],[additional]") {
  rt::test::test_schedule ts;
  const motis::station* ffm_hbf = ts.get_station("Frankfurt Hbf");
  const motis::station* Langen = ts.get_station("Langen");
  const motis::station* da_hbf = ts.get_station("Darmstadt Hbf");

  std::vector<rt::schedule_event> events = {
      rt::schedule_event(ffm_hbf->index, 999, true, t(9, 0)),
      rt::schedule_event(Langen->index, 999, false, t(9, 15)),
      rt::schedule_event(Langen->index, 999, true, t(9, 17)),
      rt::schedule_event(da_hbf->index, 999, false, t(9, 30))};

  ts._rts._message_handler.handle_additional_train(
      rt::additional_train_message("RB", events));
  ts._rts._delay_propagator.process_queue();
  ts._rts._graph_updater.finish_graph_update();

  std::vector<motis::journey> journeys =
      ts.find_connections(ffm_hbf, da_hbf, t(9, 0));
  REQUIRE(journeys.size() == 1);

  const motis::journey& j = journeys[0];
  // TODO
  CHECK(j.duration == 30 + da_hbf->transfer_time);

  ts.check_stops(j, {{ffm_hbf, t(9, 0), t(9, 0)},
                     {Langen, t(9, 15), t(9, 17)},
                     {da_hbf, t(9, 30), t(9, 30)}});

  ts.check_transports(j, {{"RB", 999, 1, 3}});

  ts.check_train(
      {{ffm_hbf, "", 0, INV, INV, "RB", 999, t(9, 0), t(9, 0)},
       {Langen, "RB", 999, t(9, 15), t(9, 15), "RB", 999, t(9, 17), t(9, 17)},
       {da_hbf, "RB", 999, t(9, 30), t(9, 30), "", 0, INV, INV}});
}

TEST_CASE("additional train with new category", "[msgs],[additional]") {
  rt::test::test_schedule ts;
  const motis::station* ffm_hbf = ts.get_station("Frankfurt Hbf");
  const motis::station* Langen = ts.get_station("Langen");
  const motis::station* da_hbf = ts.get_station("Darmstadt Hbf");
  const int h = 15;

  std::vector<rt::schedule_event> events = {
      rt::schedule_event(ffm_hbf->index, 1000, true, t(h, 0)),
      rt::schedule_event(Langen->index, 1000, false, t(h, 15)),
      rt::schedule_event(Langen->index, 1000, true, t(h, 17)),
      rt::schedule_event(da_hbf->index, 1000, false, t(h, 30))};

  ts._rts._message_handler.handle_additional_train(
      rt::additional_train_message("FOO", events));
  ts._rts._delay_propagator.process_queue();
  ts._rts._graph_updater.finish_graph_update();

  std::vector<motis::journey> journeys =
      ts.find_connections(ffm_hbf, da_hbf, t(h, 0));
  REQUIRE(journeys.size() == 1);

  const motis::journey& j = journeys[0];
  // TODO
  CHECK(j.duration == 30 + da_hbf->transfer_time);

  ts.check_stops(j, {{ffm_hbf, t(h, 0), t(h, 0)},
                     {Langen, t(h, 15), t(h, 17)},
                     {da_hbf, t(h, 30), t(h, 30)}});

  ts.check_transports(j, {{"FOO", 1000, 1, 3}});

  ts.check_train({{ffm_hbf, "", 0, INV, INV, "FOO", 1000, t(h, 0), t(h, 0)},
                  {Langen, "FOO", 1000, t(h, 15), t(h, 15), "FOO", 1000,
                   t(h, 17), t(h, 17)},
                  {da_hbf, "FOO", 1000, t(h, 30), t(h, 30), "", 0, INV, INV}});
}

TEST_CASE("cancel complete train", "[msgs],[cancel]") {
  rt::test::test_schedule ts;
  const motis::station* ffm_hbf = ts.get_station("Frankfurt Hbf");
  const motis::station* Langen = ts.get_station("Langen");
  const motis::station* da_hbf = ts.get_station("Darmstadt Hbf");

  CHECK_FALSE(ts.find_connections(da_hbf, ffm_hbf, t(13, 30)).empty());

  std::vector<rt::schedule_event> events = {
      rt::schedule_event(da_hbf->index, 20, true, t(13, 34)),
      rt::schedule_event(Langen->index, 20, false, t(13, 49)),
      rt::schedule_event(Langen->index, 20, true, t(13, 51)),
      rt::schedule_event(ffm_hbf->index, 20, false, t(14, 5))};

  ts._rts._message_handler.handle_canceled_train(
      rt::cancel_train_message(events));
  ts._rts._delay_propagator.process_queue();
  ts._rts._graph_updater.finish_graph_update();

  CHECK(ts.find_connections(da_hbf, ffm_hbf, t(13, 30)).empty());

  for (auto& e : events) {
    CHECK(ts._rts._delay_info_manager.get_delay_info(e) != nullptr);
  }
}

TEST_CASE("cancel beginning of train", "[msgs],[cancel]") {
  rt::test::test_schedule ts;
  const motis::station* ffm_hbf = ts.get_station("Frankfurt Hbf");
  const motis::station* Langen = ts.get_station("Langen");
  const motis::station* da_hbf = ts.get_station("Darmstadt Hbf");

  CHECK_FALSE(ts.find_connections(da_hbf, ffm_hbf, t(13, 30)).empty());

  std::vector<rt::schedule_event> events = {
      rt::schedule_event(da_hbf->index, 20, true, t(13, 34)),
      rt::schedule_event(Langen->index, 20, false, t(13, 49))};

  ts._rts._message_handler.handle_canceled_train(
      rt::cancel_train_message(events));
  ts._rts._delay_propagator.process_queue();
  ts._rts._graph_updater.finish_graph_update();

  CHECK(ts.find_connections(da_hbf, ffm_hbf, t(13, 30)).empty());

  std::vector<motis::journey> journeys =
      ts.find_connections(Langen, ffm_hbf, t(13, 45));
  REQUIRE(journeys.size() == 1);

  ts.check_train({{Langen, "", 0, INV, INV, "RB", 20, t(13, 51), t(13, 51)},
                  {ffm_hbf, "RB", 20, t(14, 5), t(14, 5), "", 0, INV, INV}});

  for (auto& e : events) {
    CHECK(ts._rts._delay_info_manager.get_delay_info(e) != nullptr);
  }
}

TEST_CASE("cancel end of train", "[msgs],[cancel]") {
  rt::test::test_schedule ts;
  const motis::station* ffm_hbf = ts.get_station("Frankfurt Hbf");
  const motis::station* Langen = ts.get_station("Langen");
  const motis::station* da_hbf = ts.get_station("Darmstadt Hbf");

  CHECK_FALSE(ts.find_connections(da_hbf, ffm_hbf, t(13, 30)).empty());

  std::vector<rt::schedule_event> events = {
      rt::schedule_event(Langen->index, 20, true, t(13, 51)),
      rt::schedule_event(ffm_hbf->index, 20, false, t(14, 5))};

  ts._rts._message_handler.handle_canceled_train(
      rt::cancel_train_message(events));
  ts._rts._delay_propagator.process_queue();
  ts._rts._graph_updater.finish_graph_update();

  // this connection now uses the partially canceled train
  // and then the next train (an hour later)...
  std::vector<motis::journey> journeys =
      ts.find_connections(da_hbf, ffm_hbf, t(13, 30));
  CHECK(journeys.size() == 1);
  if (!journeys.empty()) {
    const motis::journey& j = journeys[0];
    // TODO
    CHECK(j.duration == 91 + ffm_hbf->transfer_time);

    ts.check_stops(j, {{da_hbf, t(13, 34), t(13, 34)},
                       {Langen, t(13, 49), t(14, 51)},
                       {ffm_hbf, t(15, 5), t(15, 5)}});

    ts.check_transports(j, {{"RB", 20, 1, 3}});
  }

  journeys = ts.find_connections(da_hbf, Langen, t(13, 30));
  REQUIRE(journeys.size() == 1);

  ts.check_train({{da_hbf, "", 0, INV, INV, "RB", 20, t(13, 34), t(13, 34)},
                  {Langen, "RB", 20, t(13, 49), t(13, 49), "", 0, INV, INV}});

  for (auto& e : events) {
    CHECK(ts._rts._delay_info_manager.get_delay_info(e) != nullptr);
  }
}

TEST_CASE("reroute: cancel stops in the middle", "[msgs],[reroute]") {
  rt::test::test_schedule ts;
  const motis::station* ffm_tief = ts.get_station("Frankfurt Hbf Tief");
  const motis::station* dreieich = ts.get_station("Dreieich Buchschlag");
  const motis::station* Langen = ts.get_station("Langen");
  const motis::station* da_hbf = ts.get_station("Darmstadt Hbf");

  CHECK_FALSE(ts.find_connections(ffm_tief, da_hbf, t(10, 0)).empty());
  CHECK_FALSE(ts.find_connections(ffm_tief, dreieich, t(10, 0)).empty());

  std::vector<rt::schedule_event> canceled_events = {
      rt::schedule_event(dreieich->index, 1, false, t(10, 6)),
      rt::schedule_event(dreieich->index, 1, true, t(10, 8))};

  ts._rts._message_handler.handle_rerouted_train(
      rt::reroute_train_message("RE", canceled_events, {}));
  ts._rts._delay_propagator.process_queue();
  ts._rts._graph_updater.finish_graph_update();

  std::vector<motis::journey> journeys =
      ts.find_connections(ffm_tief, da_hbf, t(10, 0));
  CHECK(journeys.size() == 1);
  if (!journeys.empty()) {
    const motis::journey& j = journeys[0];

    ts.check_stops(j, {{ffm_tief, t(10, 0), t(10, 0)},
                       {Langen, t(10, 10), t(10, 17)},
                       {da_hbf, t(10, 26), t(10, 26)}});

    ts.check_transports(j, {{"RE", 1, 1, 3}});
  }

  CHECK(ts.find_connections(ffm_tief, dreieich, t(10, 0)).empty());

  ts.check_train(
      {{ffm_tief, "", 0, INV, INV, "RE", 1, t(10, 0), t(10, 0)},
       {Langen, "RE", 1, t(10, 10), t(10, 10), "RE", 1, t(10, 17), t(10, 17)},
       {da_hbf, "RE", 1, t(10, 26), t(10, 26), "", 0, INV, INV}});

  for (auto& e : canceled_events) {
    CHECK(ts._rts._delay_info_manager.get_delay_info(e) != nullptr);
  }

  // now undo the cancelation

  ts._rts._message_handler.handle_rerouted_train(
      rt::reroute_train_message("RE", {}, canceled_events));
  ts._rts._delay_propagator.process_queue();
  ts._rts._graph_updater.finish_graph_update();

  journeys = ts.find_connections(ffm_tief, da_hbf, t(10, 0));
  CHECK(journeys.size() == 1);
  if (!journeys.empty()) {
    const motis::journey& j = journeys[0];

    ts.check_stops(j, {{ffm_tief, t(10, 0), t(10, 0)},
                       {dreieich, t(10, 6), t(10, 8)},
                       {Langen, t(10, 10), t(10, 17)},
                       {da_hbf, t(10, 26), t(10, 26)}});

    ts.check_transports(j, {{"RE", 1, 1, 4}});
  }

  CHECK_FALSE(ts.find_connections(ffm_tief, dreieich, t(10, 0)).empty());

  ts.check_train(
      {{ffm_tief, "", 0, INV, INV, "RE", 1, t(10, 0), t(10, 0)},
       {dreieich, "RE", 1, t(10, 6), t(10, 6), "RE", 1, t(10, 8), t(10, 8)},
       {Langen, "RE", 1, t(10, 10), t(10, 10), "RE", 1, t(10, 17), t(10, 17)},
       {da_hbf, "RE", 1, t(10, 26), t(10, 26), "", 0, INV, INV}});
}

TEST_CASE("reroute: change start", "[msgs],[reroute]") {
  rt::test::test_schedule ts;
  const motis::station* ffm_hbf = ts.get_station("Frankfurt Hbf");
  const motis::station* ffm_tief = ts.get_station("Frankfurt Hbf Tief");
  const motis::station* dreieich = ts.get_station("Dreieich Buchschlag");
  const motis::station* Langen = ts.get_station("Langen");
  const motis::station* da_hbf = ts.get_station("Darmstadt Hbf");

  CHECK_FALSE(ts.find_connections(ffm_tief, da_hbf, t(10, 0)).empty());
  CHECK(ts.find_connections(ffm_hbf, da_hbf, t(10, 0)).empty());

  std::vector<rt::schedule_event> canceled_events = {
      rt::schedule_event(ffm_tief->index, 1, true, t(10, 0)),
  };

  std::vector<rt::schedule_event> new_events = {
      rt::schedule_event(ffm_hbf->index, 1, true, t(10, 0))};

  // delay the soon to be canceled event
  ts._rts._message_handler.handle_delay(canceled_events[0], t(10, 3),
                                        rt::timestamp_reason::FORECAST);
  ts._rts._delay_propagator.process_queue();
  ts._rts._graph_updater.finish_graph_update();

  std::vector<motis::journey> journeys =
      ts.find_connections(ffm_tief, da_hbf, t(10, 0));
  CHECK(journeys.size() == 1);
  if (!journeys.empty()) {
    const motis::journey& j = journeys[0];

    ts.check_stops(
        j, {{ffm_tief, t(10, 3), t(10, 3)},  // 10:00 -> 10:03
            {dreieich, t(10, 9), t(10, 11)},  // 10:06 -> 10:09, 10:08 -> 10:11
            {Langen, t(10, 13), t(10, 17)},  // 10:10 -> 10:13
            {da_hbf, t(10, 26), t(10, 26)}});

    ts.check_transports(j, {{"RE", 1, 1, 4}});
  }

  CHECK_FALSE(ts.find_connections(ffm_tief, dreieich, t(10, 0)).empty());

  ts.check_train(
      {{ffm_tief, "", 0, INV, INV, "RE", 1, t(10, 0), t(10, 3)},
       {dreieich, "RE", 1, t(10, 6), t(10, 9), "RE", 1, t(10, 8), t(10, 11)},
       {Langen, "RE", 1, t(10, 10), t(10, 13), "RE", 1, t(10, 17), t(10, 17)},
       {da_hbf, "RE", 1, t(10, 26), t(10, 26), "", 0, INV, INV}});

  std::cout << "\n+++++++++++++++++++++++++++++++++++++" << std::endl;
  std::cerr << "\n+++++++++++++++++++++++++++++++++++++" << std::endl;

  ts._rts._message_handler.handle_rerouted_train(
      rt::reroute_train_message("RE", canceled_events, new_events));
  ts._rts._delay_propagator.process_queue();
  ts._rts._graph_updater.finish_graph_update();

  CHECK(ts.find_connections(ffm_tief, da_hbf, t(10, 0)).empty());

  journeys = ts.find_connections(ffm_hbf, da_hbf, t(10, 0));
  CHECK(journeys.size() == 1);
  if (!journeys.empty()) {
    const motis::journey& j = journeys[0];

    ts.check_stops(j, {{ffm_hbf, t(10, 0), t(10, 0)},
                       {dreieich, t(10, 6), t(10, 8)},
                       {Langen, t(10, 10), t(10, 17)},
                       {da_hbf, t(10, 26), t(10, 26)}});

    ts.check_transports(j, {{"RE", 1, 1, 4}});
  }

  // TODO: verify
  ts.check_train(
      {{ffm_hbf, "", 0, INV, INV, "RE", 1, t(10, 0), t(10, 0)},
       {dreieich, "RE", 1, t(10, 6), t(10, 6), "RE", 1, t(10, 8), t(10, 8)},
       {Langen, "RE", 1, t(10, 10), t(10, 10), "RE", 1, t(10, 17), t(10, 17)},
       {da_hbf, "RE", 1, t(10, 26), t(10, 26), "", 0, INV, INV}});

  for (auto& e : canceled_events) {
    CHECK(ts._rts._delay_info_manager.get_delay_info(e) != nullptr);
  }

  // now undo the rerouting

  std::cout << "\n+++++++++++++++++++++++++++++++++++++" << std::endl;
  std::cerr << "\n+++++++++++++++++++++++++++++++++++++" << std::endl;

  ts._rts._message_handler.handle_rerouted_train(
      rt::reroute_train_message("RE", new_events, canceled_events));
  ts._rts._delay_propagator.process_queue();
  ts._rts._graph_updater.finish_graph_update();

  journeys = ts.find_connections(ffm_tief, da_hbf, t(10, 0));
  CHECK(journeys.size() == 1);
  if (!journeys.empty()) {
    const motis::journey& j = journeys[0];

    ts.check_stops(j, {{ffm_tief, t(10, 3), t(10, 03)},
                       {dreieich, t(10, 9), t(10, 11)},
                       {Langen, t(10, 13), t(10, 17)},
                       {da_hbf, t(10, 26), t(10, 26)}});

    ts.check_transports(j, {{"RE", 1, 1, 4}});
  }

  CHECK_FALSE(ts.find_connections(ffm_tief, dreieich, t(10, 0)).empty());

  ts.check_train(
      {{ffm_tief, "", 0, INV, INV, "RE", 1, t(10, 0), t(10, 3)},
       {dreieich, "RE", 1, t(10, 6), t(10, 9), "RE", 1, t(10, 8), t(10, 11)},
       {Langen, "RE", 1, t(10, 10), t(10, 13), "RE", 1, t(10, 17), t(10, 17)},
       {da_hbf, "RE", 1, t(10, 26), t(10, 26), "", 0, INV, INV}});
}

TEST_CASE("csd kept", "[msgs],[csd]") {
  rt::test::test_schedule ts;
  const motis::station* ffm_hbf = ts.get_station("Frankfurt Hbf");
  const motis::station* Langen = ts.get_station("Langen");
  const motis::station* da_hbf = ts.get_station("Darmstadt Hbf");
  const motis::station* off_hbf = ts.get_station("Offenbach Hbf");

  rt::schedule_event feeder_arrival(ffm_hbf->index, 20, false, t(14, 5));
  rt::schedule_event connector_departure(ffm_hbf->index, 23, true, t(14, 15));

  // check that connection works without any messages
  std::vector<motis::journey> journeys =
      ts.find_connections(da_hbf, off_hbf, t(13, 30));
  CHECK(journeys.size() == 1);
  if (!journeys.empty()) {
    const motis::journey& j = journeys[0];

    ts.check_stops(j, {{da_hbf, t(13, 34), t(13, 34)},
                       {Langen, t(13, 49), t(13, 51)},
                       {ffm_hbf, t(14, 5), t(14, 15)},
                       {off_hbf, t(14, 25), t(14, 25)}});

    ts.check_transports(j, {{"RB", 20, 1, 3}, {"RE", 23, 3, 4}});
  }

  // delay the feeder train, connection should now be broken
  ts._rts._message_handler.handle_delay(feeder_arrival, t(14, 10),
                                        rt::timestamp_reason::FORECAST);
  ts._rts._delay_propagator.process_queue();
  ts._rts._graph_updater.finish_graph_update();

  // this should use the next RE 23 which departs at 15:15
  journeys = ts.find_connections(da_hbf, off_hbf, t(13, 30));
  CHECK(journeys.size() == 1);
  if (!journeys.empty()) {
    const motis::journey& j = journeys[0];

    ts.check_stops(j, {{da_hbf, t(13, 34), t(13, 34)},
                       {Langen, t(13, 49), t(13, 51)},
                       {ffm_hbf, t(14, 10), t(15, 15)},
                       {off_hbf, t(15, 25), t(15, 25)}});

    ts.check_transports(j, {{"RB", 20, 1, 3}, {"RE", 23, 3, 4}});
  }

  // now keep the connection, RE 23 should be delayed by 3 min
  ts._rts._message_handler.handle_connection_status_decision(
      rt::connection_status_decision_message(
          feeder_arrival, connector_departure, rt::status_decision::kept));
  ts._rts._delay_propagator.process_queue();

  journeys = ts.find_connections(da_hbf, off_hbf, t(13, 30));
  CHECK(journeys.size() == 1);
  if (!journeys.empty()) {
    const motis::journey& j = journeys[0];

    ts.check_stops(j, {{da_hbf, t(13, 34), t(13, 34)},
                       {Langen, t(13, 49), t(13, 51)},
                       {ffm_hbf, t(14, 10), t(14, 18)},
                       {off_hbf, t(14, 28), t(14, 28)}});

    ts.check_transports(j, {{"RB", 20, 1, 3}, {"RE", 23, 3, 4}});
  }

  ts.check_train(
      {{da_hbf, "", 0, INV, INV, "RB", 20, t(13, 34), t(13, 34)},
       {Langen, "RB", 20, t(13, 49), t(13, 49), "RB", 20, t(13, 51), t(13, 51)},
       {ffm_hbf, "RB", 20, t(14, 5), t(14, 10), "", 0, INV, INV}});

  ts.check_train({
      {ffm_hbf, "", 0, INV, INV, "RE", 23, t(14, 15), t(14, 18)},
      {off_hbf, "RE", 23, t(14, 25), t(14, 28), "", 0, INV, INV},
  });
}
