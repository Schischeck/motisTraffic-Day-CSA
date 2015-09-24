#pragma once

#include <vector>
#include <string>
#include <tuple>
#include <memory>
#include <unordered_map>

#include "catch/catch.hpp"

#include "motis/realtime/realtime_schedule.h"
#include "motis/realtime/messages.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/station.h"
#include "motis/core/schedule/time.h"
#include "motis/loader/loader.h"
#include "motis/routing/label.h"
#include "motis/routing/search.h"
#include "motis/routing/journey.h"
#include "motis/routing/arrival.h"

#define MAX_TEST_LABELS 10000  // MAX_LABELS_WITH_MARGIN

namespace motis {
namespace realtime {
namespace test {

struct stop {
  const motis::station* station;
  struct event {
    time date_time;
  } arrival, departure;
};

struct transport {
  std::string category;
  int train_nr;
  int from, to;
};

class test_schedule {
public:
  test_schedule()
      : _schedule(motis::loader::load_schedule(
            "../modules/realtime/test/test-schedule/motis")),
        _rts(*_schedule),
        _label_store(MAX_TEST_LABELS),
        _search(*_schedule, _label_store) {
    for (const auto& s : _schedule->stations) {
      _station_map[s->name] = s.get();
    }
    //_rts._debug_mode = true;
  }

  const motis::station* get_station(std::string name) {
    const auto station = _station_map[name];
    REQUIRE(station != nullptr);
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

    motis::pareto_dijkstra::statistics stats;
    std::vector<motis::journey> journeys = _search.get_connections(
        {start}, {target}, departure_begin, departure_begin + 60, &stats);

    CHECK_FALSE(stats.max_label_quit);

    return journeys;
  }

  void check_stops(const motis::journey& journey,
                   std::vector<stop> expected_stops) {
    REQUIRE(journey.stops.size() == expected_stops.size() + 2);

    for (std::size_t i = 0; i < expected_stops.size(); i++) {
      const motis::journey::stop& jstop = journey.stops[i + 1];
      const stop& estop = expected_stops[i];
      CHECK(jstop.name == estop.station->name.to_string());
      CHECK(jstop.eva_no == estop.station->eva_nr);

      std::string exp_arrival =
          _rts._schedule.date_mgr.format_ISO(estop.arrival.date_time);
      std::string exp_departure =
          _rts._schedule.date_mgr.format_ISO(estop.departure.date_time);

      // TODO
      if (i == expected_stops.size() - 1) {
        exp_arrival = _rts._schedule.date_mgr.format_ISO(
            estop.arrival.date_time + estop.station->get_transfer_time);
        exp_departure = _rts._schedule.date_mgr.format_ISO(
            estop.departure.date_time + estop.station->get_transfer_time);
      }

      CHECK(jstop.arrival.date_time == exp_arrival);
      CHECK(jstop.departure.date_time == exp_departure);
    }
  }

  void check_transports(const motis::journey& journey,
                        std::vector<transport> expected_transports) {
    REQUIRE(journey.transports.size() == expected_transports.size() + 2);

    for (std::size_t i = 0; i < expected_transports.size(); i++) {
      const motis::journey::transport& jtransport = journey.transports[i + 1];
      const transport& etransport = expected_transports[i];
      CHECK(jtransport.name ==
            etransport.category + " " + std::to_string(etransport.train_nr));
      CHECK(jtransport.category_name == etransport.category);
      // CHECK(jtransport.train_nr == etransport.train_nr);
      CHECK(jtransport.from == etransport.from);
      CHECK(jtransport.to == etransport.to);
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
    REQUIRE(start_event.found());
    CHECK(start_event == first_event);

    std::vector<std::tuple<node*, schedule_event, schedule_event>> events =
        _rts.get_train_events(start_event);

    auto cts = stops.begin();

    for (auto e : events) {
      CHECK(cts != stops.end());

      node* route_node;
      schedule_event sdep, sarr;
      std::tie(route_node, sarr, sdep) = e;
      graph_event gdep = _rts.get_graph_event(sdep);
      graph_event garr = _rts.get_graph_event(sarr);

      CHECK(route_node->get_station()->_id == cts->station->index);

      CHECK(schedule_event(
                cts->scheduled_a_time == INVALID_TIME ? 0 : cts->station->index,
                cts->a_train_nr, false, cts->scheduled_a_time) == sarr);
      CHECK(schedule_event(
                cts->scheduled_d_time == INVALID_TIME ? 0 : cts->station->index,
                cts->d_train_nr, true, cts->scheduled_d_time) == sdep);

      CHECK(graph_event(
                cts->real_a_time == INVALID_TIME ? 0 : cts->station->index,
                cts->a_train_nr, false, cts->real_a_time,
                garr._route_id) == garr);
      CHECK(graph_event(
                cts->real_d_time == INVALID_TIME ? 0 : cts->station->index,
                cts->d_train_nr, true, cts->real_d_time,
                gdep._route_id) == gdep);

      light_connection* lc;
      if (garr.found()) {
        std::tie(std::ignore, lc) = _rts.locate_event(garr);
        REQUIRE(lc != nullptr);
        CHECK(cts->a_category ==
              _rts._schedule.category_names[lc->_full_con->con_info->family]);
      }
      if (gdep.found()) {
        std::tie(std::ignore, lc) = _rts.locate_event(gdep);
        REQUIRE(lc != nullptr);
        CHECK(cts->d_category ==
              _rts._schedule.category_names[lc->_full_con->con_info->family]);
      }

      ++cts;
    }

    CHECK(cts == stops.end());
  }

  std::unique_ptr<motis::schedule> _schedule;
  motis::realtime::realtime_schedule _rts;
  motis::memory_manager<motis::label> _label_store;
  motis::search _search;
  std::unordered_map<std::string, const motis::station*> _station_map;
};

}  // namespace test
}  // namespace realtime
}  // namespace motis
