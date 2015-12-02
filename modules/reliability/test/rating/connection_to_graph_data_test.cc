#include "gtest/gtest.h"

#include "motis/core/common/date_util.h"
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
  schedule_station const ERLANGEN = {"Erlangen", "0953067"};
  schedule_station const FRANKFURT = {"Frankfurt", "5744986"};
  schedule_station const KASSEL = {"Kassel", "6380201"};
  schedule_station const STUTTGART = {"Stuttgart", "7309882"};
  unsigned short const ICE_E_K = 7;  // 12:45 --> 14:15
  unsigned short const ICE_S_E = 5;  // 11:32 --> 12:32
  unsigned short const ICE_K_F_S = 3;  // 09:15 --> 10:15, 10:20 --> 11:15
};
class reliability_connection_to_graph_data5 : public test_motis_setup {
public:
  reliability_connection_to_graph_data5()
      : test_motis_setup("modules/reliability/resources/schedule5/",
                         "20151019") {}
  schedule_station const DARMSTADT = {"Darmstadt", "1111111"};
  schedule_station const FRANKFURT = {"Frankfurt", "2222222"};
  schedule_station const GIESSEN = {"Giessen", "3333333"};
  schedule_station const MARBURG = {"Marburg", "4444444"};
  unsigned short const RE_D_F_G = 1;  // 08:00 --> 08:20, 08:22 --> 09:00
  unsigned short const RE_G_M = 2;  // 09:10 --> 09:40
};
class reliability_connection_to_graph_data6 : public test_motis_setup {
public:
  reliability_connection_to_graph_data6()
      : test_motis_setup(
            "modules/reliability/resources/schedule6_footconnection/",
            "20151019") {}
  schedule_station const MANNHEIM = {"Mannheim", "2222222"};
  schedule_station const DARMSTADT = {"Darmstadt", "3333333"};
  schedule_station const TUD = {"TUD", "4444444"};
  schedule_station const FRANKFURT = {"Frankfurt", "5555555"};
  schedule_station const HAUPTWACHE = {"Hauptwache", "6666666"};
  unsigned short const IC_M_D = 1;  // 08:10 --> 08:40
  unsigned short const RE_T_F = 2;  // 08:45 --> 09:15
};

TEST_F(reliability_connection_to_graph_data2, to_element) {
  // route node at Frankfurt of train ICE_S_E
  auto& first_route_node =
      *graph_accessor::get_first_route_node(get_schedule(), ICE_S_E);
  // route edge from Stuttgart to Erlangen
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& first_light_conn = first_route_edge->_m._route_edge._conns[0];

  auto const element_ice_s_e = detail::to_element(
      2, get_schedule(), STUTTGART.eva, ERLANGEN.eva,
      test_util::minutes_to_motis_time(11 * 60 + 32),
      test_util::minutes_to_motis_time(12 * 60 + 32), first_route_node._route,
      graph_accessor::find_family(get_schedule().categories, "ICE").second,
      ICE_S_E, "");

  ASSERT_TRUE(element_ice_s_e.departure_stop_idx_ == 2);
  ASSERT_TRUE(element_ice_s_e.arrival_stop_idx() == 3);
  ASSERT_TRUE(element_ice_s_e.from_ == first_route_edge->_from);
  ASSERT_TRUE(element_ice_s_e.to_ == first_route_edge->_to);
  ASSERT_TRUE(element_ice_s_e.is_first_route_node_);
  ASSERT_TRUE(element_ice_s_e.light_connection_ == &first_light_conn);
  ASSERT_TRUE(element_ice_s_e.light_connection_idx_ == 0);
}

TEST_F(reliability_connection_to_graph_data2, to_element2) {
  // route node at Frankfurt of train ICE_K_F_S
  auto& route_node =
      *graph_accessor::get_departing_route_edge(
           *graph_accessor::get_first_route_node(get_schedule(), ICE_K_F_S))
           ->_to;
  // route edge from Frankfurt to Stuttgart
  auto const route_edge = graph_accessor::get_departing_route_edge(route_node);
  auto const& first_light_conn = route_edge->_m._route_edge._conns[0];

  auto const element_ice_k_f_s = detail::to_element(
      3, get_schedule(), FRANKFURT.eva, STUTTGART.eva,
      test_util::minutes_to_motis_time(10 * 60 + 20),
      test_util::minutes_to_motis_time(11 * 60 + 15), route_node._route,
      graph_accessor::find_family(get_schedule().categories, "ICE").second,
      ICE_K_F_S, "");

  ASSERT_TRUE(element_ice_k_f_s.departure_stop_idx_ == 3);
  ASSERT_TRUE(element_ice_k_f_s.arrival_stop_idx() == 4);
  ASSERT_TRUE(element_ice_k_f_s.from_ == route_edge->_from);
  ASSERT_TRUE(element_ice_k_f_s.to_ == route_edge->_to);
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
  auto req_msg = flatbuffers::request_builder::to_routing_request(
      STUTTGART.name, STUTTGART.eva, KASSEL.name, KASSEL.eva,
      (motis::time)(11 * 60 + 27), (motis::time)(11 * 60 + 27),
      std::make_tuple(28, 9, 2015), false);
  auto msg = bootstrap::send(motis_instance_, req_msg);
  ASSERT_TRUE(msg);
  auto const journeys =
      message_to_journeys(msg->content<routing::RoutingResponse const*>());
  ASSERT_EQ(1, journeys.size());

  auto const elements = get_elements(get_schedule(), journeys.front());
  ASSERT_TRUE(elements.size() == 2);
  {
    ASSERT_TRUE(elements[0].size() == 1);
    auto const element = elements[0][0];
    ASSERT_TRUE(element.departure_stop_idx_ ==
                1);  // note: connections begin with a dummy walk
    ASSERT_TRUE(element.arrival_stop_idx() == 2);
    ASSERT_TRUE(
        get_schedule().stations[element.from_->_station_node->_id]->eva_nr ==
        STUTTGART.eva);
    ASSERT_TRUE(
        get_schedule().stations[element.to_->_station_node->_id]->eva_nr ==
        ERLANGEN.eva);
    ASSERT_TRUE(element.light_connection_->d_time ==
                test_util::minutes_to_motis_time(11 * 60 + 32));
    ASSERT_TRUE(element.light_connection_->a_time ==
                test_util::minutes_to_motis_time(12 * 60 + 32));
    ASSERT_TRUE(element.light_connection_->_full_con->con_info->train_nr ==
                ICE_S_E);
  }
  {
    ASSERT_TRUE(elements[1].size() == 1);
    auto const element = elements[1][0];
    ASSERT_TRUE(element.departure_stop_idx_ == 2);
    ASSERT_TRUE(element.arrival_stop_idx() == 3);
    ASSERT_TRUE(
        get_schedule().stations[element.from_->_station_node->_id]->eva_nr ==
        ERLANGEN.eva);
    ASSERT_TRUE(
        get_schedule().stations[element.to_->_station_node->_id]->eva_nr ==
        KASSEL.eva);
    ASSERT_TRUE(element.light_connection_->d_time ==
                test_util::minutes_to_motis_time(12 * 60 + 45));
    ASSERT_TRUE(element.light_connection_->a_time ==
                test_util::minutes_to_motis_time(14 * 60 + 15));
    ASSERT_TRUE(element.light_connection_->_full_con->con_info->train_nr ==
                ICE_E_K);
  }

  auto const element = get_last_element(get_schedule(), journeys.front());
  test_element(elements[1][0], element);
}

TEST_F(reliability_connection_to_graph_data5, get_elements2) {
  auto req_msg = flatbuffers::request_builder::to_routing_request(
      DARMSTADT.name, DARMSTADT.eva, MARBURG.name, MARBURG.eva,
      (motis::time)(7 * 60 + 55), (motis::time)(8 * 60 + 5),
      std::make_tuple(19, 10, 2015), false);
  auto msg = bootstrap::send(motis_instance_, req_msg);
  ASSERT_TRUE(msg);
  auto const journeys =
      message_to_journeys(msg->content<routing::RoutingResponse const*>());
  ASSERT_EQ(1, journeys.size());

  auto const elements = get_elements(get_schedule(), journeys.front());
  ASSERT_TRUE(elements.size() == 2);
  {
    ASSERT_TRUE(elements.at(0).size() == 2);
    {
      auto const element = elements[0][0];
      ASSERT_TRUE(element.departure_stop_idx_ ==
                  1);  // note: connections begin with a dummy walk
      ASSERT_TRUE(element.arrival_stop_idx() == 2);
      ASSERT_TRUE(
          get_schedule().stations[element.from_->_station_node->_id]->eva_nr ==
          DARMSTADT.eva);
      ASSERT_TRUE(
          get_schedule().stations[element.to_->_station_node->_id]->eva_nr ==
          FRANKFURT.eva);
      ASSERT_TRUE(element.light_connection_->d_time ==
                  test_util::minutes_to_motis_time(8 * 60));
      ASSERT_TRUE(element.light_connection_->a_time ==
                  test_util::minutes_to_motis_time(8 * 60 + 20));
      ASSERT_TRUE(element.light_connection_->_full_con->con_info->train_nr ==
                  RE_D_F_G);
    }
    {
      auto const element = elements[0][1];
      ASSERT_TRUE(element.departure_stop_idx_ == 2);
      ASSERT_TRUE(element.arrival_stop_idx() == 3);
      ASSERT_TRUE(
          get_schedule().stations[element.from_->_station_node->_id]->eva_nr ==
          FRANKFURT.eva);
      ASSERT_TRUE(
          get_schedule().stations[element.to_->_station_node->_id]->eva_nr ==
          GIESSEN.eva);
      ASSERT_TRUE(element.light_connection_->d_time ==
                  test_util::minutes_to_motis_time(8 * 60 + 22));
      ASSERT_TRUE(element.light_connection_->a_time ==
                  test_util::minutes_to_motis_time(9 * 60));
      ASSERT_TRUE(element.light_connection_->_full_con->con_info->train_nr ==
                  RE_D_F_G);
    }
  }
  {
    ASSERT_TRUE(elements.at(1).size() == 1);
    auto const element = elements[1][0];
    ASSERT_TRUE(element.departure_stop_idx_ == 3);
    ASSERT_TRUE(element.arrival_stop_idx() == 4);
    ASSERT_TRUE(
        get_schedule().stations[element.from_->_station_node->_id]->eva_nr ==
        GIESSEN.eva);
    ASSERT_TRUE(
        get_schedule().stations[element.to_->_station_node->_id]->eva_nr ==
        MARBURG.eva);
    ASSERT_TRUE(element.light_connection_->d_time ==
                test_util::minutes_to_motis_time(9 * 60 + 10));
    ASSERT_TRUE(element.light_connection_->a_time ==
                test_util::minutes_to_motis_time(9 * 60 + 40));
    ASSERT_TRUE(element.light_connection_->_full_con->con_info->train_nr ==
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
  auto req_msg = flatbuffers::request_builder::to_routing_request(
      MANNHEIM.name, MANNHEIM.eva, HAUPTWACHE.name, HAUPTWACHE.eva,
      (motis::time)(8 * 60 + 10), (motis::time)(8 * 60 + 11),
      std::make_tuple(19, 10, 2015), false);
  auto msg = bootstrap::send(motis_instance_, req_msg);

  ASSERT_TRUE(msg);
  auto const journeys =
      message_to_journeys(msg->content<routing::RoutingResponse const*>());
  ASSERT_EQ(1, journeys.size());

  auto const elements = get_elements(get_schedule(), journeys.front());
  ASSERT_TRUE(elements.size() == 2);
  {
    ASSERT_TRUE(elements[0].size() == 1);
    auto const element = elements[0][0];
    ASSERT_TRUE(element.departure_stop_idx_ == 1);
    ASSERT_TRUE(element.arrival_stop_idx() == 2);
    ASSERT_TRUE(
        get_schedule().stations[element.from_->_station_node->_id]->eva_nr ==
        MANNHEIM.eva);
    ASSERT_TRUE(
        get_schedule().stations[element.to_->_station_node->_id]->eva_nr ==
        DARMSTADT.eva);
    ASSERT_TRUE(element.light_connection_->d_time ==
                test_util::minutes_to_motis_time(8 * 60 + 10));
    ASSERT_TRUE(element.light_connection_->a_time ==
                test_util::minutes_to_motis_time(8 * 60 + 40));
    ASSERT_TRUE(element.light_connection_->_full_con->con_info->train_nr ==
                IC_M_D);
  }
  {
    ASSERT_TRUE(elements[1].size() == 1);
    auto const element = elements[1][0];
    ASSERT_TRUE(element.departure_stop_idx_ == 3);
    ASSERT_TRUE(element.arrival_stop_idx() == 4);
    ASSERT_TRUE(
        get_schedule().stations[element.from_->_station_node->_id]->eva_nr ==
        TUD.eva);
    ASSERT_TRUE(
        get_schedule().stations[element.to_->_station_node->_id]->eva_nr ==
        FRANKFURT.eva);
    ASSERT_TRUE(element.light_connection_->d_time ==
                test_util::minutes_to_motis_time(8 * 60 + 45));
    ASSERT_TRUE(element.light_connection_->a_time ==
                test_util::minutes_to_motis_time(9 * 60 + 15));
    ASSERT_TRUE(element.light_connection_->_full_con->con_info->train_nr ==
                RE_T_F);
  }

  auto const element = get_last_element(get_schedule(), journeys.front());
  test_element(elements[1][0], element);
}

}  // namespace connection_to_graph_data
}  // namespace rating
}  // namespace reliability
}  // namespace motis
