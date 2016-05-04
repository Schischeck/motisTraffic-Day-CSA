#include "gtest/gtest.h"

#include "motis/core/common/date_time_util.h"
#include "motis/core/journey/journey.h"
#include "motis/core/journey/message_to_journeys.h"

#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

#include "motis/loader/loader.h"

#include "motis/routing/routing.h"

#include "motis/reliability/computation/distributions_calculator.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/rating/connection_rating.h"
#include "motis/reliability/rating/connection_to_graph_data.h"
#include "motis/reliability/tools/flatbuffers/request_builder.h"

#include "../include/test_schedule_setup.h"
#include "../include/test_util.h"

namespace motis {
namespace reliability {
namespace rating {
namespace connection_to_graph_data {

class reliability_connection_to_graph_data2 : public test_motis_setup {
public:
  reliability_connection_to_graph_data2()
      : test_motis_setup("modules/reliability/resources/schedule2/",
                         "20150928") {}
  constexpr static schedule_station ERLANGEN = {"Erlangen", "0953067"};
  constexpr static schedule_station FRANKFURT = {"Frankfurt", "5744986"};
  constexpr static schedule_station KASSEL = {"Kassel", "6380201"};
  constexpr static schedule_station STUTTGART = {"Stuttgart", "7309882"};
  constexpr static unsigned ICE_E_K = 7;  // 12:45 --> 14:15
  constexpr static unsigned ICE_S_E = 5;  // 11:32 --> 12:32
  constexpr static unsigned ICE_K_F_S = 3;  // 09:15 --> 10:15, 10:20 --> 11:15
};
class reliability_connection_to_graph_data5 : public test_motis_setup {
public:
  reliability_connection_to_graph_data5()
      : test_motis_setup("modules/reliability/resources/schedule5/",
                         "20151019") {}
  constexpr static schedule_station DARMSTADT = {"Darmstadt", "1111111"};
  constexpr static schedule_station FRANKFURT = {"Frankfurt", "2222222"};
  constexpr static schedule_station GIESSEN = {"Giessen", "3333333"};
  constexpr static schedule_station MARBURG = {"Marburg", "4444444"};
  constexpr static unsigned RE_D_F_G = 1;  // 08:00 --> 08:20, 08:22 --> 09:00
  constexpr static unsigned RE_G_M = 2;  // 09:10 --> 09:40
};
class reliability_connection_to_graph_data6 : public test_motis_setup {
public:
  reliability_connection_to_graph_data6()
      : test_motis_setup(
            "modules/reliability/resources/schedule6_footconnection/",
            "20151019") {}
  constexpr static schedule_station MANNHEIM = {"Mannheim", "2222222"};
  constexpr static schedule_station DARMSTADT = {"Darmstadt", "3333333"};
  constexpr static schedule_station TUD = {"TUD", "4444444"};
  constexpr static schedule_station FRANKFURT = {"Frankfurt", "5555555"};
  constexpr static schedule_station HAUPTWACHE = {"Hauptwache", "6666666"};
  constexpr static unsigned IC_M_D = 1;  // 08:10 --> 08:40
  constexpr static unsigned RE_T_F = 2;  // 08:45 --> 09:15
};

TEST_F(reliability_connection_to_graph_data2, to_element) {
  // route node at Frankfurt of train ICE_S_E
  auto& first_route_node =
      *graph_accessor::get_first_route_node(get_schedule(), ICE_S_E);
  // route edge from Stuttgart to Erlangen
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& first_light_conn = first_route_edge->m_.route_edge_.conns_[0];

  auto const element_ice_s_e = detail::to_element(
      2, get_schedule(), STUTTGART.eva, ERLANGEN.eva,
      test_util::minutes_to_motis_time(11 * 60 + 32),
      test_util::minutes_to_motis_time(12 * 60 + 32),
      graph_accessor::find_family(get_schedule().categories_, "ICE").second,
      ICE_S_E, "");

  ASSERT_TRUE(element_ice_s_e.departure_stop_idx_ == 2);
  ASSERT_TRUE(element_ice_s_e.arrival_stop_idx() == 3);
  ASSERT_TRUE(element_ice_s_e.from_ == first_route_edge->from_);
  ASSERT_TRUE(element_ice_s_e.to_ == first_route_edge->to_);
  ASSERT_TRUE(element_ice_s_e.is_first_route_node_);
  ASSERT_TRUE(element_ice_s_e.light_connection_ == &first_light_conn);
  ASSERT_TRUE(element_ice_s_e.light_connection_idx_ == 0);
}

TEST_F(reliability_connection_to_graph_data2, to_element2) {
  // route node at Frankfurt of train ICE_K_F_S
  auto& route_node =
      *graph_accessor::get_departing_route_edge(
           *graph_accessor::get_first_route_node(get_schedule(), ICE_K_F_S))
           ->to_;
  // route edge from Frankfurt to Stuttgart
  auto const route_edge = graph_accessor::get_departing_route_edge(route_node);
  auto const& first_light_conn = route_edge->m_.route_edge_.conns_[0];

  auto const element_ice_k_f_s = detail::to_element(
      3, get_schedule(), FRANKFURT.eva, STUTTGART.eva,
      test_util::minutes_to_motis_time(10 * 60 + 20),
      test_util::minutes_to_motis_time(11 * 60 + 15),
      graph_accessor::find_family(get_schedule().categories_, "ICE").second,
      ICE_K_F_S, "");

  ASSERT_TRUE(element_ice_k_f_s.departure_stop_idx_ == 3);
  ASSERT_TRUE(element_ice_k_f_s.arrival_stop_idx() == 4);
  ASSERT_TRUE(element_ice_k_f_s.from_ == route_edge->from_);
  ASSERT_TRUE(element_ice_k_f_s.to_ == route_edge->to_);
  ASSERT_FALSE(element_ice_k_f_s.is_first_route_node_);
  ASSERT_TRUE(element_ice_k_f_s.light_connection_ == &first_light_conn);
  ASSERT_TRUE(element_ice_k_f_s.light_connection_idx_ == 0);
}

void test_element(connection_element const& expected,
                  connection_element const& element) {
  ASSERT_EQ(expected.departure_stop_idx_, element.departure_stop_idx_);
  ASSERT_EQ(expected.from_, element.from_);
  ASSERT_EQ(expected.is_first_route_node_, element.is_first_route_node_);
  ASSERT_EQ(expected.light_connection_, element.light_connection_);
  ASSERT_EQ(expected.light_connection_idx_, element.light_connection_idx_);
  ASSERT_EQ(expected.to_, element.to_);
}

TEST_F(reliability_connection_to_graph_data2, get_elements) {
  auto req_msg =
      flatbuffers::request_builder()
          .add_pretrip_start(STUTTGART.name, STUTTGART.eva,
                             test_util::hhmm_to_unixtime(get_schedule(), 1132),
                             test_util::hhmm_to_unixtime(get_schedule(), 1132))
          .add_destination(KASSEL.name, KASSEL.eva)
          .build_routing_request();
  auto msg = test::call(motis_instance_, req_msg);
  using routing::RoutingResponse;
  auto const journeys =
      message_to_journeys(motis_content(RoutingResponse, msg));

  ASSERT_EQ(1, journeys.size());

  auto const elements = get_elements(get_schedule(), journeys.front());
  ASSERT_TRUE(elements.size() == 2);
  {
    ASSERT_TRUE(elements[0].size() == 1);
    auto const element = elements[0][0];
    ASSERT_EQ(0, element.departure_stop_idx_);
    ASSERT_TRUE(element.arrival_stop_idx() == 1);
    ASSERT_TRUE(
        get_schedule().stations_[element.from_->station_node_->id_]->eva_nr_ ==
        STUTTGART.eva);
    ASSERT_TRUE(
        get_schedule().stations_[element.to_->station_node_->id_]->eva_nr_ ==
        ERLANGEN.eva);
    ASSERT_TRUE(element.light_connection_->d_time_ ==
                test_util::minutes_to_motis_time(11 * 60 + 32));
    ASSERT_TRUE(element.light_connection_->a_time_ ==
                test_util::minutes_to_motis_time(12 * 60 + 32));
    ASSERT_TRUE(element.light_connection_->full_con_->con_info_->train_nr_ ==
                ICE_S_E);
  }
  {
    ASSERT_TRUE(elements[1].size() == 1);
    auto const element = elements[1][0];
    ASSERT_TRUE(element.departure_stop_idx_ == 1);
    ASSERT_TRUE(element.arrival_stop_idx() == 2);
    ASSERT_TRUE(
        get_schedule().stations_[element.from_->station_node_->id_]->eva_nr_ ==
        ERLANGEN.eva);
    ASSERT_TRUE(
        get_schedule().stations_[element.to_->station_node_->id_]->eva_nr_ ==
        KASSEL.eva);
    ASSERT_TRUE(element.light_connection_->d_time_ ==
                test_util::minutes_to_motis_time(12 * 60 + 45));
    ASSERT_TRUE(element.light_connection_->a_time_ ==
                test_util::minutes_to_motis_time(14 * 60 + 15));
    ASSERT_TRUE(element.light_connection_->full_con_->con_info_->train_nr_ ==
                ICE_E_K);
  }

  auto const element = get_last_element(get_schedule(), journeys.front());
  test_element(elements[1][0], element);
}

TEST_F(reliability_connection_to_graph_data5, get_elements2) {
  auto req_msg =
      flatbuffers::request_builder()
          .add_pretrip_start(DARMSTADT.name, DARMSTADT.eva,
                             test_util::hhmm_to_unixtime(get_schedule(), 755),
                             test_util::hhmm_to_unixtime(get_schedule(), 805))
          .add_destination(MARBURG.name, MARBURG.eva)
          .build_routing_request();
  auto msg = test::call(motis_instance_, req_msg);
  using routing::RoutingResponse;
  auto const journeys =
      message_to_journeys(motis_content(RoutingResponse, msg));

  ASSERT_EQ(1, journeys.size());

  auto const elements = get_elements(get_schedule(), journeys.front());
  ASSERT_TRUE(elements.size() == 2);
  {
    ASSERT_TRUE(elements.at(0).size() == 2);
    {
      auto const element = elements[0][0];
      ASSERT_EQ(0, element.departure_stop_idx_);
      ASSERT_TRUE(element.arrival_stop_idx() == 1);
      ASSERT_TRUE(get_schedule()
                      .stations_[element.from_->station_node_->id_]
                      ->eva_nr_ == DARMSTADT.eva);
      ASSERT_TRUE(
          get_schedule().stations_[element.to_->station_node_->id_]->eva_nr_ ==
          FRANKFURT.eva);
      ASSERT_TRUE(element.light_connection_->d_time_ ==
                  test_util::minutes_to_motis_time(8 * 60));
      ASSERT_TRUE(element.light_connection_->a_time_ ==
                  test_util::minutes_to_motis_time(8 * 60 + 20));
      ASSERT_TRUE(element.light_connection_->full_con_->con_info_->train_nr_ ==
                  RE_D_F_G);
    }
    {
      auto const element = elements[0][1];
      ASSERT_TRUE(element.departure_stop_idx_ == 1);
      ASSERT_TRUE(element.arrival_stop_idx() == 2);
      ASSERT_TRUE(get_schedule()
                      .stations_[element.from_->station_node_->id_]
                      ->eva_nr_ == FRANKFURT.eva);
      ASSERT_TRUE(
          get_schedule().stations_[element.to_->station_node_->id_]->eva_nr_ ==
          GIESSEN.eva);
      ASSERT_TRUE(element.light_connection_->d_time_ ==
                  test_util::minutes_to_motis_time(8 * 60 + 22));
      ASSERT_TRUE(element.light_connection_->a_time_ ==
                  test_util::minutes_to_motis_time(9 * 60));
      ASSERT_TRUE(element.light_connection_->full_con_->con_info_->train_nr_ ==
                  RE_D_F_G);
    }
  }
  {
    ASSERT_TRUE(elements.at(1).size() == 1);
    auto const element = elements[1][0];
    ASSERT_TRUE(element.departure_stop_idx_ == 2);
    ASSERT_TRUE(element.arrival_stop_idx() == 3);
    ASSERT_TRUE(
        get_schedule().stations_[element.from_->station_node_->id_]->eva_nr_ ==
        GIESSEN.eva);
    ASSERT_TRUE(
        get_schedule().stations_[element.to_->station_node_->id_]->eva_nr_ ==
        MARBURG.eva);
    ASSERT_TRUE(element.light_connection_->d_time_ ==
                test_util::minutes_to_motis_time(9 * 60 + 10));
    ASSERT_TRUE(element.light_connection_->a_time_ ==
                test_util::minutes_to_motis_time(9 * 60 + 40));
    ASSERT_TRUE(element.light_connection_->full_con_->con_info_->train_nr_ ==
                RE_G_M);
  }

  auto const element = get_last_element(get_schedule(), journeys.front());
  test_element(elements[1][0], element);
}

/* Connection with two walking sections:
 * Mannheim --> Darmstadt with IC_M_D,
 * Darmstadt --> TUD via walking,
 * TUD --> Frankfurt with RE_T_F, and
 * Frankfurt --> Hauptwache via walking.
 * (Note: the routing-module does not deliver connections
 * with a station-to-station walking at the beginning).
 */
TEST_F(reliability_connection_to_graph_data6, get_elements_foot) {
  auto req_msg =
      flatbuffers::request_builder()
          .add_pretrip_start(MANNHEIM.name, MANNHEIM.eva,
                             test_util::hhmm_to_unixtime(get_schedule(), 810),
                             test_util::hhmm_to_unixtime(get_schedule(), 811))
          .add_destination(HAUPTWACHE.name, HAUPTWACHE.eva)
          .build_routing_request();
  auto msg = test::call(motis_instance_, req_msg);
  using routing::RoutingResponse;
  auto const journeys =
      message_to_journeys(motis_content(RoutingResponse, msg));

  ASSERT_EQ(1, journeys.size());

  auto const elements = get_elements(get_schedule(), journeys.front());
  ASSERT_TRUE(elements.size() == 2);
  {
    ASSERT_TRUE(elements[0].size() == 1);
    auto const element = elements[0][0];
    ASSERT_TRUE(element.departure_stop_idx_ == 0);
    ASSERT_TRUE(element.arrival_stop_idx() == 1);
    ASSERT_TRUE(
        get_schedule().stations_[element.from_->station_node_->id_]->eva_nr_ ==
        MANNHEIM.eva);
    ASSERT_TRUE(
        get_schedule().stations_[element.to_->station_node_->id_]->eva_nr_ ==
        DARMSTADT.eva);
    ASSERT_TRUE(element.light_connection_->d_time_ ==
                test_util::minutes_to_motis_time(8 * 60 + 10));
    ASSERT_TRUE(element.light_connection_->a_time_ ==
                test_util::minutes_to_motis_time(8 * 60 + 40));
    ASSERT_TRUE(element.light_connection_->full_con_->con_info_->train_nr_ ==
                IC_M_D);
  }
  {
    ASSERT_TRUE(elements[1].size() == 1);
    auto const element = elements[1][0];
    ASSERT_TRUE(element.departure_stop_idx_ == 2);
    ASSERT_TRUE(element.arrival_stop_idx() == 3);
    ASSERT_TRUE(
        get_schedule().stations_[element.from_->station_node_->id_]->eva_nr_ ==
        TUD.eva);
    ASSERT_TRUE(
        get_schedule().stations_[element.to_->station_node_->id_]->eva_nr_ ==
        FRANKFURT.eva);
    ASSERT_TRUE(element.light_connection_->d_time_ ==
                test_util::minutes_to_motis_time(8 * 60 + 45));
    ASSERT_TRUE(element.light_connection_->a_time_ ==
                test_util::minutes_to_motis_time(9 * 60 + 15));
    ASSERT_TRUE(element.light_connection_->full_con_->con_info_->train_nr_ ==
                RE_T_F);
  }

  auto const element = get_last_element(get_schedule(), journeys.front());
  test_element(elements[1][0], element);
}

}  // namespace connection_to_graph_data
}  // namespace rating
}  // namespace reliability
}  // namespace motis
