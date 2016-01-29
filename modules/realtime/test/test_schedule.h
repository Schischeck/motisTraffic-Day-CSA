#pragma once

#include <vector>
#include <string>
#include <tuple>
#include <memory>
#include <unordered_map>
#include <iostream>

#include "gtest/gtest.h"

#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/station.h"
#include "motis/core/schedule/time.h"
#include "motis/core/common/date_util.h"
#include "motis/core/journey/journey.h"
#include "motis/loader/loader.h"
#include "motis/routing/search.h"
#include "motis/routing/arrival.h"
#include "motis/realtime/realtime_schedule.h"
#include "motis/realtime/messages.h"

namespace motis {
namespace realtime {
namespace test {

struct stop {
  const motis::station* station;
  struct event {
    motis::time date_time;
  } arrival, departure;
};

struct transport {
  std::string category;
  int train_nr;
  int from, to;
};

class test_schedule : public ::testing::Test {
public:
  test_schedule()
      : _schedule(motis::loader::load_schedule(
            {"modules/realtime/test/test-schedule", true, false, true, false,
             "20150126", 21})),
        _rts(*_schedule),
        _label_store((size_t)32 * 1042 * 1024),
        _search(*_schedule, _label_store) {
    for (const auto& s : _schedule->stations) {
      _station_map[s->name] = s.get();
    }
    // _rts._debug_mode = true;
  }

  const motis::station* get_station(std::string name) {
    const auto station = _station_map[name];
    // ASSERT_TRUE(station != nullptr);
    return station;
  }

  std::vector<motis::journey> find_connections(const motis::station* from,
                                               const motis::station* to,
                                               motis::time departure_begin,
                                               int interval = 60) {
    motis::arrival_part start;
    start.station = from->index;
    motis::arrival_part target;
    target.station = to->index;

    return _search.get_connections({start}, {target}, departure_begin,
                                   departure_begin + interval, true)
        .journeys;
  }

  void check_stops(const motis::journey& journey,
                   std::vector<stop> expected_stops) {
    ASSERT_EQ(expected_stops.size() + 1, journey.stops.size());

    for (std::size_t i = 0; i < expected_stops.size(); i++) {
      const motis::journey::stop& jstop = journey.stops[i];
      const stop& estop = expected_stops[i];

      EXPECT_EQ(estop.station->name, jstop.name);
      EXPECT_EQ(estop.station->eva_nr, jstop.eva_no);

      auto exp_arrival = estop.arrival.date_time;
      auto exp_departure = estop.departure.date_time;

      if (i == expected_stops.size() - 1) {
        exp_departure = exp_arrival + estop.station->transfer_time;
      }

      EXPECT_EQ(exp_arrival,
                motis::unix_to_motistime(_schedule->schedule_begin_,
                                         jstop.arrival.timestamp));
      EXPECT_EQ(exp_departure,
                motis::unix_to_motistime(_schedule->schedule_begin_,
                                         jstop.departure.timestamp));
    }
  }

  void check_transports(const motis::journey& journey,
                        std::vector<transport> expected_transports) {
    ASSERT_EQ(expected_transports.size() + 1, journey.transports.size());

    for (std::size_t i = 0; i < expected_transports.size(); i++) {
      const motis::journey::transport& jtransport = journey.transports[i];
      const transport& etransport = expected_transports[i];
      EXPECT_EQ(etransport.category + " " + std::to_string(etransport.train_nr),
                jtransport.name);
      EXPECT_EQ(etransport.category, jtransport.category_name);
      // EXPECT_EQ(jtransport.train_nr == etransport.train_nr);
      EXPECT_EQ(etransport.from, jtransport.from);
      EXPECT_EQ(etransport.to, jtransport.to);
    }
  }

  struct check_train_stop {
    const motis::station* station;
    std::string a_category;
    uint32_t a_train_nr;
    motis::time scheduled_a_time;
    motis::time real_a_time;
    std::string d_category;
    uint32_t d_train_nr;
    motis::time scheduled_d_time;
    motis::time real_d_time;
  };

  void check_train(const std::vector<check_train_stop>& stops) {
    using namespace motis;
    using namespace motis::realtime;
    if (stops.empty()) return;
    const check_train_stop& first_stop = stops[0];
    schedule_event first_event(first_stop.station->index, first_stop.d_train_nr,
                               true, first_stop.scheduled_d_time);
    schedule_event start_event;
    std::tie(start_event, std::ignore, std::ignore, std::ignore) =
        _rts.locate_start_of_train(first_event);
    ASSERT_TRUE(start_event.found());
    EXPECT_EQ(first_event, start_event);

    std::vector<std::tuple<node*, schedule_event, schedule_event> > events =
        _rts.get_train_events(start_event);

    auto cts = stops.begin();

    for (auto e : events) {
      ASSERT_TRUE(cts != stops.end());

      node* route_node;
      schedule_event sdep, sarr;
      std::tie(route_node, sarr, sdep) = e;
      graph_event gdep = _rts.get_graph_event(sdep);
      graph_event garr = _rts.get_graph_event(sarr);

      EXPECT_EQ(cts->station->index, route_node->get_station()->_id);

      EXPECT_EQ(
          schedule_event(
              cts->scheduled_a_time == INVALID_TIME ? 0 : cts->station->index,
              cts->a_train_nr, false, cts->scheduled_a_time),
          sarr);
      EXPECT_EQ(
          schedule_event(
              cts->scheduled_d_time == INVALID_TIME ? 0 : cts->station->index,
              cts->d_train_nr, true, cts->scheduled_d_time),
          sdep);

      EXPECT_EQ(graph_event(
                    cts->real_a_time == INVALID_TIME ? 0 : cts->station->index,
                    cts->a_train_nr, false, cts->real_a_time, garr._route_id),
                garr);
      EXPECT_EQ(graph_event(
                    cts->real_d_time == INVALID_TIME ? 0 : cts->station->index,
                    cts->d_train_nr, true, cts->real_d_time, gdep._route_id),
                gdep);

      light_connection* lc;
      if (garr.found()) {
        std::tie(std::ignore, lc) = _rts.locate_event(garr);
        ASSERT_TRUE(lc != nullptr);
        EXPECT_EQ(
            cts->a_category,
            _rts._schedule.categories[lc->_full_con->con_info->family]->name);
      }
      if (gdep.found()) {
        std::tie(std::ignore, lc) = _rts.locate_event(gdep);
        ASSERT_TRUE(lc != nullptr);
        EXPECT_EQ(
            cts->d_category,
            _rts._schedule.categories[lc->_full_con->con_info->family]->name);
      }

      ++cts;
    }

    ASSERT_TRUE(cts == stops.end());
  }

  std::unique_ptr<motis::schedule> _schedule;
  motis::realtime::realtime_schedule _rts;
  motis::routing::memory_manager _label_store;
  motis::routing::search _search;
  std::unordered_map<std::string, const motis::station*> _station_map;
};

}  // namespace test
}  // namespace realtime
}  // namespace motis
