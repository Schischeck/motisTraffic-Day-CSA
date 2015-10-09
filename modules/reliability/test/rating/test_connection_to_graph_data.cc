#include "catch/catch.hpp"

#include "motis/core/common/date_util.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

#include "motis/loader/loader.h"

#include "motis/routing/routing.h"

#include "motis/reliability/distributions_calculator.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/rating/connection_to_graph_data.h"
#include "motis/reliability/tools/flatbuffers_tools.h"
#include "motis/reliability/tools/system.h"

using namespace motis;
using namespace motis::reliability;
using namespace motis::reliability::rating::connection_to_graph_data;

namespace schedule2 {
struct station {
  std::string name;
  std::string eva;
};
station const ERLANGEN = {"Erlangen", "0953067"};
station const FRANKFURT = {"Frankfurt", "5744986"};
station const KASSEL = {"Kassel", "6380201"};
station const STUTTGART = {"Stuttgart", "7309882"};
short const ICE_E_K = 7;  // 12:45 --> 14:15
short const ICE_S_E = 5;  // 11:32 --> 12:32
short const ICE_K_F_S = 3;  // 09:15 --> 10:15, 10:20 --> 11:15
}

namespace test_connection_to_graph_data {
auto schedule =
    loader::load_schedule("../modules/reliability/resources/schedule2/",
                          to_unix_time(2015, 9, 28), to_unix_time(2015, 9, 29));
}

TEST_CASE("to_element", "[conn_to_graph_data]") {
  auto const element_ice_s_e =
      detail::to_element(*test_connection_to_graph_data::schedule,
                         schedule2::STUTTGART.eva, schedule2::ERLANGEN.eva,
                         11 * 60 + 32, 12 * 60 + 32, "ICE", schedule2::ICE_S_E);

  // route node at Frankfurt of train ICE_S_E
  auto& first_route_node =
      *graph_accessor::get_first_route_node(
          *test_connection_to_graph_data::schedule, schedule2::ICE_S_E);
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
  auto const element_ice_k_f_s = detail::to_element(
      *test_connection_to_graph_data::schedule, schedule2::FRANKFURT.eva,
      schedule2::STUTTGART.eva, 10 * 60 + 20, 11 * 60 + 15, "ICE",
      schedule2::ICE_K_F_S);

  // route node at Frankfurt of train ICE_K_F_S
  auto& route_node =
      *graph_accessor::get_departing_route_edge(
           *graph_accessor::get_first_route_node(
               *test_connection_to_graph_data::schedule, schedule2::ICE_K_F_S))
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
  auto& schedule = *test_connection_to_graph_data::schedule;
  system_tools::setup setup(&schedule);

  auto msg = flatbuffers_tools::to_flatbuffers_message(
      schedule2::STUTTGART.name, schedule2::STUTTGART.eva,
      schedule2::KASSEL.name, schedule2::KASSEL.eva,
      (motis::time)(11 * 60 + 30), (motis::time)(11 * 60 + 35));

  auto test_cb = [=](motis::module::msg_ptr msg, boost::system::error_code e) {
    auto response = msg->content<routing::RoutingResponse const*>();
    REQUIRE(response->connections()->size() == 1);

    auto const elements = get_elements(*test_connection_to_graph_data::schedule,
                                       *response->connections()->begin());
    REQUIRE(elements.size() == 2);
    REQUIRE(test_connection_to_graph_data::schedule
                ->stations[elements[0].from_->_station_node->_id]
                ->eva_nr == schedule2::STUTTGART.eva);
    REQUIRE(test_connection_to_graph_data::schedule
                ->stations[elements[0].to_->_station_node->_id]
                ->eva_nr == schedule2::ERLANGEN.eva);
    REQUIRE(elements[0].light_connection_->d_time == 11 * 60 + 32);
    REQUIRE(elements[0].light_connection_->a_time == 12 * 60 + 32);
    REQUIRE(elements[0].light_connection_->_full_con->con_info->train_nr ==
            schedule2::ICE_S_E);

    REQUIRE(test_connection_to_graph_data::schedule
                ->stations[elements[1].from_->_station_node->_id]
                ->eva_nr == schedule2::ERLANGEN.eva);
    REQUIRE(test_connection_to_graph_data::schedule
                ->stations[elements[1].to_->_station_node->_id]
                ->eva_nr == schedule2::KASSEL.eva);
    REQUIRE(elements[1].light_connection_->d_time == 12 * 60 + 45);
    REQUIRE(elements[1].light_connection_->a_time == 14 * 60 + 15);
    REQUIRE(elements[1].light_connection_->_full_con->con_info->train_nr ==
            schedule2::ICE_E_K);
  };

  setup.dispatcher.on_msg(msg, 0, test_cb);
  setup.ios.run();
}
