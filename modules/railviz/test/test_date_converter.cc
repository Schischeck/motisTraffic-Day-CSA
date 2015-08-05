#include <iostream>
#include <memory>

#include "catch/catch.hpp"

#include "motis/loader/loader.h"
#include "motis/railviz/date_converter.h"

using namespace motis;
using namespace motis::railviz;

/**
 * The Tests operate on ../test_timetables/date_manager_test1/motis (relative to
 *build path)
 *
 * There is one single Train defined that drives every day three times from
 *station A to station B.
 * #    departure-time  arrival-time
 * 1    7:56            8:00
 * 2    8:56            9:00
 * 3    9:56            10:00
 */

bool railviz_date_converter_simple_test_check_dtime(std::time_t* time) {
  std::tm* ptm = std::localtime(time);
  if (ptm->tm_hour == 7 || ptm->tm_hour == 8 || ptm->tm_hour == 9)
    if (ptm->tm_min == 56)
      if (ptm->tm_sec == 0) {
        return true;
      }
  return false;
}

bool railviz_date_converter_simple_test_check_atime(std::time_t* time) {
  std::tm* ptm = std::localtime(time);
  if (ptm->tm_hour == 8 || ptm->tm_hour == 9 || ptm->tm_hour == 10)
    if (ptm->tm_min == 0)
      if (ptm->tm_sec == 0) {
        return true;
      }
  return false;
}

TEST_CASE("simple_time_test", "[railviz]") {
  auto schedule =
      loader::load_schedule("../test_timetables/date_manager_test1/motis");
  railviz::date_converter dcnv(schedule.get()->date_mgr);
  date_manager& mgr = schedule.get()->date_mgr;

  for (auto const& station_node : schedule->station_nodes) {
    for (auto const& route_node : station_node->get_route_nodes()) {
      for (auto const& edge : route_node->_edges) {
        if (edge.type() != edge::ROUTE_EDGE) continue;
        for (auto const& lcon : edge._m._route_edge._conns) {
          std::time_t d_time = dcnv.convert(lcon.d_time);
          std::time_t a_time = dcnv.convert(lcon.a_time);
          REQUIRE(railviz_date_converter_simple_test_check_dtime(&d_time));
          REQUIRE(railviz_date_converter_simple_test_check_atime(&a_time));
        }
      }
    }
  }
}
