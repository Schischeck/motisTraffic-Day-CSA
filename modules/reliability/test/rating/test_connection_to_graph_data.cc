#include "catch/catch.hpp"

#include "motis/core/common/date_util.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

#include "motis/loader/loader.h"

#include "motis/routing/routing.h"

#include "motis/reliability/distributions_calculator.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/rating/connection_to_graph_data.h"
#include "motis/reliability/tools/system.h"

using namespace motis;
using namespace motis::reliability;
using namespace motis::reliability::rating::connection_to_graph_data;

namespace schedule2 {
std::string const ERLANGEN = "0953067";
std::string const FRANKFURT = "5744986";
std::string const KASSEL = "6380201";
std::string const STUTTGART = "7309882";
short const ICE_S_E = 5;  // 11:32 --> 12:32
short const ICE_K_F_S = 3;  // 09:15 --> 10:15, 10:20 --> 11:15
}

TEST_CASE("to_element", "[conn_to_graph_data]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule2/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));

  auto const element_ice_s_e =
      detail::to_element(*schedule, schedule2::STUTTGART, schedule2::ERLANGEN,
                         11 * 60 + 32, 12 * 60 + 32, "ICE", schedule2::ICE_S_E);

  // route node at Frankfurt of train ICE_S_E
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule, schedule2::ICE_S_E);
  // route edge from Stuttgart to Erlangen
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& first_light_conn = first_route_edge->_m._route_edge._conns[0];

  REQUIRE(element_ice_s_e.from_ == first_route_edge->_from);
  REQUIRE(element_ice_s_e.to_ == first_route_edge->_to);
  REQUIRE(element_ice_s_e.is_first_route_node_);
  REQUIRE(element_ice_s_e.light_connection_ == &first_light_conn);
  REQUIRE(element_ice_s_e.light_connection_idx_ == 0);
}

TEST_CASE("to_element2", "[conn_to_graph_data]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule2/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));

  auto const element_ice_k_f_s = detail::to_element(
      *schedule, schedule2::FRANKFURT, schedule2::STUTTGART, 10 * 60 + 20,
      11 * 60 + 15, "ICE", schedule2::ICE_K_F_S);

  // route node at Frankfurt of train ICE_K_F_S
  auto& route_node = *graph_accessor::get_departing_route_edge(
                          *graph_accessor::get_first_route_node(
                              *schedule, schedule2::ICE_K_F_S))
                          ->_to;
  // route edge from Frankfurt to Stuttgart
  auto const route_edge = graph_accessor::get_departing_route_edge(route_node);
  auto const& first_light_conn = route_edge->_m._route_edge._conns[0];

  REQUIRE(element_ice_k_f_s.from_ == route_edge->_from);
  REQUIRE(element_ice_k_f_s.to_ == route_edge->_to);
  REQUIRE_FALSE(element_ice_k_f_s.is_first_route_node_);
  REQUIRE(element_ice_k_f_s.light_connection_ == &first_light_conn);
  REQUIRE(element_ice_k_f_s.light_connection_idx_ == 0);
}

TEST_CASE("get_elements", "[conn_to_graph_data]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule2/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));
  system_tools::setup setup(schedule.get());
}
