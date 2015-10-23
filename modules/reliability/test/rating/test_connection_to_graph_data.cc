#include "catch/catch.hpp"

#include "motis/core/common/date_util.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

#include "motis/loader/loader.h"

#include "motis/routing/routing.h"

#include "motis/reliability/computation/distributions_calculator.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/rating/connection_rating.h"
#include "motis/reliability/rating/connection_to_graph_data.h"
#include "motis/reliability/tools/flatbuffers_tools.h"
#include "motis/reliability/tools/system.h"

using namespace motis;
using namespace motis::reliability;
using namespace motis::reliability::rating::connection_to_graph_data;

struct schedule_station {
  std::string name;
  std::string eva;
};

namespace schedule2 {
schedule_station const ERLANGEN = {"Erlangen", "0953067"};
schedule_station const FRANKFURT = {"Frankfurt", "5744986"};
schedule_station const KASSEL = {"Kassel", "6380201"};
schedule_station const STUTTGART = {"Stuttgart", "7309882"};
short const ICE_E_K = 7;  // 12:45 --> 14:15
short const ICE_S_E = 5;  // 11:32 --> 12:32
short const ICE_K_F_S = 3;  // 09:15 --> 10:15, 10:20 --> 11:15
}
namespace schedule5 {
schedule_station const DARMSTADT = {"Darmstadt", "1111111"};
schedule_station const FRANKFURT = {"Frankfurt", "2222222"};
schedule_station const GIESSEN = {"Giessen", "3333333"};
schedule_station const MARBURG = {"Marburg", "4444444"};
short const RE_D_F_G = 1;  // 08:00 --> 08:20, 08:22 --> 09:00
short const RE_G_M = 2;  // 09:10 --> 09:40
}
namespace schedule6 {
schedule_station const MANNHEIM = {"Mannheim", "2222222"};
schedule_station const DARMSTADT = {"Darmstadt", "3333333"};
schedule_station const TUD = {"TUD", "4444444"};
schedule_station const FRANKFURT = {"Frankfurt", "5555555"};
schedule_station const HAUPTWACHE = {"Hauptwache", "6666666"};
short const IC_M_D = 1;  // 08:10 --> 08:40
short const RE_T_F = 2;  // 08:45 --> 09:15
}

namespace test_connection_to_graph_data {
auto schedule2 =
    loader::load_schedule("modules/reliability/resources/schedule2/",
                          to_unix_time(2015, 9, 28), to_unix_time(2015, 9, 29));
auto schedule5 = loader::load_schedule(
    "modules/reliability/resources/schedule5/", to_unix_time(2015, 10, 19),
    to_unix_time(2015, 10, 20));
auto schedule6 = loader::load_schedule(
    "modules/reliability/resources/schedule6_footconnection/",
    to_unix_time(2015, 10, 19), to_unix_time(2015, 10, 20));
}

TEST_CASE("to_element", "[conn_to_graph_data]") {
  auto const element_ice_s_e =
      detail::to_element(2, *test_connection_to_graph_data::schedule2,
                         schedule2::STUTTGART.eva, schedule2::ERLANGEN.eva,
                         11 * 60 + 32, 12 * 60 + 32, "ICE", schedule2::ICE_S_E);

  // route node at Frankfurt of train ICE_S_E
  auto& first_route_node =
      *graph_accessor::get_first_route_node(
          *test_connection_to_graph_data::schedule2, schedule2::ICE_S_E);
  // route edge from Stuttgart to Erlangen
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& first_light_conn = first_route_edge->_m._route_edge._conns[0];

  REQUIRE(element_ice_s_e.departure_stop_idx_ == 2);
  REQUIRE(element_ice_s_e.arrival_stop_idx() == 3);
  REQUIRE(element_ice_s_e.from_ == first_route_edge->_from);
  REQUIRE(element_ice_s_e.to_ == first_route_edge->_to);
  REQUIRE(element_ice_s_e.is_first_route_node_);
  REQUIRE(element_ice_s_e.light_connection_ == &first_light_conn);
  REQUIRE(element_ice_s_e.light_connection_idx_ == 0);
}

TEST_CASE("to_element2", "[conn_to_graph_data]") {
  auto const element_ice_k_f_s = detail::to_element(
      3, *test_connection_to_graph_data::schedule2, schedule2::FRANKFURT.eva,
      schedule2::STUTTGART.eva, 10 * 60 + 20, 11 * 60 + 15, "ICE",
      schedule2::ICE_K_F_S);

  // route node at Frankfurt of train ICE_K_F_S
  auto& route_node = *graph_accessor::get_departing_route_edge(
                          *graph_accessor::get_first_route_node(
                              *test_connection_to_graph_data::schedule2,
                              schedule2::ICE_K_F_S))->_to;
  // route edge from Frankfurt to Stuttgart
  auto const route_edge = graph_accessor::get_departing_route_edge(route_node);
  auto const& first_light_conn = route_edge->_m._route_edge._conns[0];

  REQUIRE(element_ice_k_f_s.departure_stop_idx_ == 3);
  REQUIRE(element_ice_k_f_s.arrival_stop_idx() == 4);
  REQUIRE(element_ice_k_f_s.from_ == route_edge->_from);
  REQUIRE(element_ice_k_f_s.to_ == route_edge->_to);
  REQUIRE_FALSE(element_ice_k_f_s.is_first_route_node_);
  REQUIRE(element_ice_k_f_s.light_connection_ == &first_light_conn);
  REQUIRE(element_ice_k_f_s.light_connection_idx_ == 0);
}

TEST_CASE("get_elements", "[conn_to_graph_data]") {
  system_tools::setup setup(test_connection_to_graph_data::schedule2.get());

  auto msg = flatbuffers_tools::to_routing_request(
      schedule2::STUTTGART.name, schedule2::STUTTGART.eva,
      schedule2::KASSEL.name, schedule2::KASSEL.eva,
      (motis::time)(11 * 60 + 30), (motis::time)(11 * 60 + 35),
      std::make_tuple(28, 9, 2015));
  auto test_cb = [&](motis::module::msg_ptr msg, boost::system::error_code e) {
    REQUIRE(msg);
    auto response = msg->content<routing::RoutingResponse const*>();
    REQUIRE(response->connections()->size() == 1);

    auto const elements =
        get_elements(*test_connection_to_graph_data::schedule2,
                     *response->connections()->begin()).second;
    REQUIRE(elements.size() == 2);
    {
      REQUIRE(elements[0].size() == 1);
      auto const element = elements[0][0];
      REQUIRE(element.departure_stop_idx_ ==
              1);  // note: connections begin with a dummy walk
      REQUIRE(element.arrival_stop_idx() == 2);
      REQUIRE(test_connection_to_graph_data::schedule2
                  ->stations[element.from_->_station_node->_id]
                  ->eva_nr == schedule2::STUTTGART.eva);
      REQUIRE(test_connection_to_graph_data::schedule2
                  ->stations[element.to_->_station_node->_id]
                  ->eva_nr == schedule2::ERLANGEN.eva);
      REQUIRE(element.light_connection_->d_time == 11 * 60 + 32);
      REQUIRE(element.light_connection_->a_time == 12 * 60 + 32);
      REQUIRE(element.light_connection_->_full_con->con_info->train_nr ==
              schedule2::ICE_S_E);
    }
    {
      REQUIRE(elements[1].size() == 1);
      auto const element = elements[1][0];
      REQUIRE(element.departure_stop_idx_ == 2);
      REQUIRE(element.arrival_stop_idx() == 3);
      REQUIRE(test_connection_to_graph_data::schedule2
                  ->stations[element.from_->_station_node->_id]
                  ->eva_nr == schedule2::ERLANGEN.eva);
      REQUIRE(test_connection_to_graph_data::schedule2
                  ->stations[element.to_->_station_node->_id]
                  ->eva_nr == schedule2::KASSEL.eva);
      REQUIRE(element.light_connection_->d_time == 12 * 60 + 45);
      REQUIRE(element.light_connection_->a_time == 14 * 60 + 15);
      REQUIRE(element.light_connection_->_full_con->con_info->train_nr ==
              schedule2::ICE_E_K);
    }
  };
  setup.dispatcher.on_msg(msg, 0, test_cb);
  setup.ios.run();
}

TEST_CASE("get_elements2", "[conn_to_graph_data]") {
  system_tools::setup setup(test_connection_to_graph_data::schedule5.get());
  auto msg = flatbuffers_tools::to_routing_request(
      schedule5::DARMSTADT.name, schedule5::DARMSTADT.eva,
      schedule5::MARBURG.name, schedule5::MARBURG.eva,
      (motis::time)(7 * 60 + 55), (motis::time)(8 * 60 + 5),
      std::make_tuple(19, 10, 2015));

  auto test_cb = [&](motis::module::msg_ptr msg, boost::system::error_code e) {
    REQUIRE(msg);
    auto response = msg->content<routing::RoutingResponse const*>();
    REQUIRE(response->connections()->size() == 1);

    auto const elements =
        get_elements(*test_connection_to_graph_data::schedule5,
                     *response->connections()->begin()).second;
    REQUIRE(elements.size() == 2);
    {
      REQUIRE(elements.at(0).size() == 2);
      {
        auto const element = elements[0][0];
        REQUIRE(element.departure_stop_idx_ ==
                1);  // note: connections begin with a dummy walk
        REQUIRE(element.arrival_stop_idx() == 2);
        REQUIRE(test_connection_to_graph_data::schedule5
                    ->stations[element.from_->_station_node->_id]
                    ->eva_nr == schedule5::DARMSTADT.eva);
        REQUIRE(test_connection_to_graph_data::schedule5
                    ->stations[element.to_->_station_node->_id]
                    ->eva_nr == schedule5::FRANKFURT.eva);
        REQUIRE(element.light_connection_->d_time == 8 * 60);
        REQUIRE(element.light_connection_->a_time == 8 * 60 + 20);
        REQUIRE(element.light_connection_->_full_con->con_info->train_nr ==
                schedule5::RE_D_F_G);
      }
      {
        auto const element = elements[0][1];
        REQUIRE(element.departure_stop_idx_ == 2);
        REQUIRE(element.arrival_stop_idx() == 3);
        REQUIRE(test_connection_to_graph_data::schedule5
                    ->stations[element.from_->_station_node->_id]
                    ->eva_nr == schedule5::FRANKFURT.eva);
        REQUIRE(test_connection_to_graph_data::schedule5
                    ->stations[element.to_->_station_node->_id]
                    ->eva_nr == schedule5::GIESSEN.eva);
        REQUIRE(element.light_connection_->d_time == 8 * 60 + 22);
        REQUIRE(element.light_connection_->a_time == 9 * 60);
        REQUIRE(element.light_connection_->_full_con->con_info->train_nr ==
                schedule5::RE_D_F_G);
      }
    }
    {
      REQUIRE(elements.at(1).size() == 1);
      auto const element = elements[1][0];
      REQUIRE(element.departure_stop_idx_ == 3);
      REQUIRE(element.arrival_stop_idx() == 4);
      REQUIRE(test_connection_to_graph_data::schedule5
                  ->stations[element.from_->_station_node->_id]
                  ->eva_nr == schedule5::GIESSEN.eva);
      REQUIRE(test_connection_to_graph_data::schedule5
                  ->stations[element.to_->_station_node->_id]
                  ->eva_nr == schedule5::MARBURG.eva);
      REQUIRE(element.light_connection_->d_time == 9 * 60 + 10);
      REQUIRE(element.light_connection_->a_time == 9 * 60 + 40);
      REQUIRE(element.light_connection_->_full_con->con_info->train_nr ==
              schedule5::RE_G_M);
    }
  };
  setup.dispatcher.on_msg(msg, 0, test_cb);
  setup.ios.run();
}

/* Connection with two walking sections:
 * Mannheim --> Darmstadt with IC_M_D,
 * Darmstadt --> TUD via walking,
 * TUD --> Frankfurt with RE_T_F, and
 * Frankfurt --> Hauptwache via walking.
 * (Note: the routing-module does not deliver connections
 * with a station-to-station walking at the beginning).
 */
TEST_CASE("get_elements_foot", "[conn_to_graph_data]") {
  system_tools::setup setup(test_connection_to_graph_data::schedule6.get());
  auto msg = flatbuffers_tools::to_routing_request(
      schedule6::MANNHEIM.name, schedule6::MANNHEIM.eva,
      schedule6::HAUPTWACHE.name, schedule6::HAUPTWACHE.eva,
      (motis::time)(8 * 60 + 10), (motis::time)(8 * 60 + 11),
      std::make_tuple(19, 10, 2015));

  auto test_cb = [&](motis::module::msg_ptr msg, boost::system::error_code e) {
    REQUIRE(msg);
    auto response = msg->content<routing::RoutingResponse const*>();
    REQUIRE(response->connections()->size() == 1);

    auto const elements =
        get_elements(*test_connection_to_graph_data::schedule6,
                     *response->connections()->begin()).second;
    REQUIRE(elements.size() == 2);
    {
      REQUIRE(elements[0].size() == 1);
      auto const element = elements[0][0];
      REQUIRE(element.departure_stop_idx_ == 1);
      REQUIRE(element.arrival_stop_idx() == 2);
      REQUIRE(test_connection_to_graph_data::schedule6
                  ->stations[element.from_->_station_node->_id]
                  ->eva_nr == schedule6::MANNHEIM.eva);
      REQUIRE(test_connection_to_graph_data::schedule6
                  ->stations[element.to_->_station_node->_id]
                  ->eva_nr == schedule6::DARMSTADT.eva);
      REQUIRE(element.light_connection_->d_time == 8 * 60 + 10);
      REQUIRE(element.light_connection_->a_time == 8 * 60 + 40);
      REQUIRE(element.light_connection_->_full_con->con_info->train_nr ==
              schedule6::IC_M_D);
    }
    {
      REQUIRE(elements[1].size() == 1);
      auto const element = elements[1][0];
      REQUIRE(element.departure_stop_idx_ == 3);
      REQUIRE(element.arrival_stop_idx() == 4);
      REQUIRE(test_connection_to_graph_data::schedule6
                  ->stations[element.from_->_station_node->_id]
                  ->eva_nr == schedule6::TUD.eva);
      REQUIRE(test_connection_to_graph_data::schedule6
                  ->stations[element.to_->_station_node->_id]
                  ->eva_nr == schedule6::FRANKFURT.eva);
      REQUIRE(element.light_connection_->d_time == 8 * 60 + 45);
      REQUIRE(element.light_connection_->a_time == 9 * 60 + 15);
      REQUIRE(element.light_connection_->_full_con->con_info->train_nr ==
              schedule6::RE_T_F);
    }
  };
  setup.dispatcher.on_msg(msg, 0, test_cb);
  setup.ios.run();
}
