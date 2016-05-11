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

#include "../include/schedules/schedule2.h"
#include "../include/schedules/schedule5.h"
#include "../include/schedules/schedule6.h"
#include "../include/test_schedule_setup.h"
#include "../include/test_util.h"

namespace motis {
namespace reliability {
namespace rating {
namespace connection_to_graph_data {

class reliability_connection_to_graph_data2 : public test_motis_setup {
public:
  reliability_connection_to_graph_data2()
      : test_motis_setup(schedule2::PATH, schedule2::DATE) {}
};
class reliability_connection_to_graph_data5 : public test_motis_setup {
public:
  reliability_connection_to_graph_data5()
      : test_motis_setup(schedule5::PATH, schedule5::DATE) {}
};
class reliability_connection_to_graph_data6 : public test_motis_setup {
public:
  reliability_connection_to_graph_data6()
      : test_motis_setup(schedule6::PATH, schedule6::DATE) {}
};

TEST_F(reliability_connection_to_graph_data2, to_element) {
  // route node at Frankfurt of train ICE_S_E
  auto& first_route_node =
      *graph_accessor::get_first_route_node(get_schedule(), schedule2::ICE_S_E);
  // route edge from Stuttgart to Erlangen
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& first_light_conn = first_route_edge->m_.route_edge_.conns_[0];

  auto const element_ice_s_e = detail::to_element(
      2, get_schedule(), schedule2::STUTTGART.eva_, schedule2::ERLANGEN.eva_,
      test_util::minutes_to_motis_time(11 * 60 + 32),
      test_util::minutes_to_motis_time(12 * 60 + 32),
      graph_accessor::find_family(get_schedule().categories_, "ICE").second,
      schedule2::ICE_S_E, "");

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
  auto& route_node = *graph_accessor::get_departing_route_edge(
                          *graph_accessor::get_first_route_node(
                              get_schedule(), schedule2::ICE_K_F_S))
                          ->to_;
  // route edge from Frankfurt to Stuttgart
  auto const route_edge = graph_accessor::get_departing_route_edge(route_node);
  auto const& first_light_conn = route_edge->m_.route_edge_.conns_[0];

  auto const element_ice_k_f_s = detail::to_element(
      3, get_schedule(), schedule2::FRANKFURT.eva_, schedule2::STUTTGART.eva_,
      test_util::minutes_to_motis_time(10 * 60 + 20),
      test_util::minutes_to_motis_time(11 * 60 + 15),
      graph_accessor::find_family(get_schedule().categories_, "ICE").second,
      schedule2::ICE_K_F_S, "");

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
          .add_pretrip_start(schedule2::STUTTGART.name_,
                             schedule2::STUTTGART.eva_,
                             test_util::hhmm_to_unixtime(get_schedule(), 1132),
                             test_util::hhmm_to_unixtime(get_schedule(), 1132))
          .add_destination(schedule2::KASSEL.name_, schedule2::KASSEL.eva_)
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
        schedule2::STUTTGART.eva_);
    ASSERT_TRUE(
        get_schedule().stations_[element.to_->station_node_->id_]->eva_nr_ ==
        schedule2::ERLANGEN.eva_);
    ASSERT_TRUE(element.light_connection_->d_time_ ==
                test_util::minutes_to_motis_time(11 * 60 + 32));
    ASSERT_TRUE(element.light_connection_->a_time_ ==
                test_util::minutes_to_motis_time(12 * 60 + 32));
    ASSERT_TRUE(element.light_connection_->full_con_->con_info_->train_nr_ ==
                schedule2::ICE_S_E);
  }
  {
    ASSERT_TRUE(elements[1].size() == 1);
    auto const element = elements[1][0];
    ASSERT_TRUE(element.departure_stop_idx_ == 1);
    ASSERT_TRUE(element.arrival_stop_idx() == 2);
    ASSERT_TRUE(
        get_schedule().stations_[element.from_->station_node_->id_]->eva_nr_ ==
        schedule2::ERLANGEN.eva_);
    ASSERT_TRUE(
        get_schedule().stations_[element.to_->station_node_->id_]->eva_nr_ ==
        schedule2::KASSEL.eva_);
    ASSERT_TRUE(element.light_connection_->d_time_ ==
                test_util::minutes_to_motis_time(12 * 60 + 45));
    ASSERT_TRUE(element.light_connection_->a_time_ ==
                test_util::minutes_to_motis_time(14 * 60 + 15));
    ASSERT_TRUE(element.light_connection_->full_con_->con_info_->train_nr_ ==
                schedule2::ICE_E_K);
  }

  auto const element = get_last_element(get_schedule(), journeys.front());
  test_element(elements[1][0], element);
}

TEST_F(reliability_connection_to_graph_data5, get_elements2) {
  auto req_msg =
      flatbuffers::request_builder()
          .add_pretrip_start(schedule5::DARMSTADT.name_,
                             schedule5::DARMSTADT.eva_,
                             test_util::hhmm_to_unixtime(get_schedule(), 755),
                             test_util::hhmm_to_unixtime(get_schedule(), 805))
          .add_destination(schedule5::MARBURG.name_, schedule5::MARBURG.eva_)
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
                      ->eva_nr_ == schedule5::DARMSTADT.eva_);
      ASSERT_TRUE(
          get_schedule().stations_[element.to_->station_node_->id_]->eva_nr_ ==
          schedule5::FRANKFURT.eva_);
      ASSERT_TRUE(element.light_connection_->d_time_ ==
                  test_util::minutes_to_motis_time(8 * 60));
      ASSERT_TRUE(element.light_connection_->a_time_ ==
                  test_util::minutes_to_motis_time(8 * 60 + 20));
      ASSERT_TRUE(element.light_connection_->full_con_->con_info_->train_nr_ ==
                  schedule5::RE_D_F_G);
    }
    {
      auto const element = elements[0][1];
      ASSERT_TRUE(element.departure_stop_idx_ == 1);
      ASSERT_TRUE(element.arrival_stop_idx() == 2);
      ASSERT_TRUE(get_schedule()
                      .stations_[element.from_->station_node_->id_]
                      ->eva_nr_ == schedule5::FRANKFURT.eva_);
      ASSERT_TRUE(
          get_schedule().stations_[element.to_->station_node_->id_]->eva_nr_ ==
          schedule5::GIESSEN.eva_);
      ASSERT_TRUE(element.light_connection_->d_time_ ==
                  test_util::minutes_to_motis_time(8 * 60 + 22));
      ASSERT_TRUE(element.light_connection_->a_time_ ==
                  test_util::minutes_to_motis_time(9 * 60));
      ASSERT_TRUE(element.light_connection_->full_con_->con_info_->train_nr_ ==
                  schedule5::RE_D_F_G);
    }
  }
  {
    ASSERT_TRUE(elements.at(1).size() == 1);
    auto const element = elements[1][0];
    ASSERT_TRUE(element.departure_stop_idx_ == 2);
    ASSERT_TRUE(element.arrival_stop_idx() == 3);
    ASSERT_TRUE(
        get_schedule().stations_[element.from_->station_node_->id_]->eva_nr_ ==
        schedule5::GIESSEN.eva_);
    ASSERT_TRUE(
        get_schedule().stations_[element.to_->station_node_->id_]->eva_nr_ ==
        schedule5::MARBURG.eva_);
    ASSERT_TRUE(element.light_connection_->d_time_ ==
                test_util::minutes_to_motis_time(9 * 60 + 10));
    ASSERT_TRUE(element.light_connection_->a_time_ ==
                test_util::minutes_to_motis_time(9 * 60 + 40));
    ASSERT_TRUE(element.light_connection_->full_con_->con_info_->train_nr_ ==
                schedule5::RE_G_M);
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
  auto req_msg = flatbuffers::request_builder()
                     .add_pretrip_start(
                         schedule6::MANNHEIM.name_, schedule6::MANNHEIM.eva_,
                         test_util::hhmm_to_unixtime(get_schedule(), 810),
                         test_util::hhmm_to_unixtime(get_schedule(), 811))
                     .add_destination(schedule6::HAUPTWACHE.name_,
                                      schedule6::HAUPTWACHE.eva_)
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
        schedule6::MANNHEIM.eva_);
    ASSERT_TRUE(
        get_schedule().stations_[element.to_->station_node_->id_]->eva_nr_ ==
        schedule6::DARMSTADT.eva_);
    ASSERT_TRUE(element.light_connection_->d_time_ ==
                test_util::minutes_to_motis_time(8 * 60 + 10));
    ASSERT_TRUE(element.light_connection_->a_time_ ==
                test_util::minutes_to_motis_time(8 * 60 + 40));
    ASSERT_TRUE(element.light_connection_->full_con_->con_info_->train_nr_ ==
                schedule6::IC_M_D);
  }
  {
    ASSERT_TRUE(elements[1].size() == 1);
    auto const element = elements[1][0];
    ASSERT_TRUE(element.departure_stop_idx_ == 2);
    ASSERT_TRUE(element.arrival_stop_idx() == 3);
    ASSERT_TRUE(
        get_schedule().stations_[element.from_->station_node_->id_]->eva_nr_ ==
        schedule6::TUD.eva_);
    ASSERT_TRUE(
        get_schedule().stations_[element.to_->station_node_->id_]->eva_nr_ ==
        schedule6::FRANKFURT.eva_);
    ASSERT_TRUE(element.light_connection_->d_time_ ==
                test_util::minutes_to_motis_time(8 * 60 + 45));
    ASSERT_TRUE(element.light_connection_->a_time_ ==
                test_util::minutes_to_motis_time(9 * 60 + 15));
    ASSERT_TRUE(element.light_connection_->full_con_->con_info_->train_nr_ ==
                schedule6::RE_T_F);
  }

  auto const element = get_last_element(get_schedule(), journeys.front());
  test_element(elements[1][0], element);
}

}  // namespace connection_to_graph_data
}  // namespace rating
}  // namespace reliability
}  // namespace motis
