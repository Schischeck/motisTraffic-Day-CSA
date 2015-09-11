#include "catch/catch.hpp"

#include "motis/loader/loader.h"

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/distributions_container.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/computation/data_departure_interchange.h"

#include "include/precomputed_distributions_test_container.h"
#include "include/start_and_travel_test_distributions.h"

using namespace motis;
using namespace motis::reliability;
using namespace motis::reliability::calc_departure_distribution;

namespace schedule2 {
short const KASSEL = 1;
short const FRANKFURT = 2;
short const STUTTGART = 3;
short const ERLANGEN = 4;

short const RE_K_F = 0;
short const ICE_F_S = 1;
short const S_F_S = 2;
short const ICE_S_E = 3;
short const S_S_E = 4;
short const ICE_K_F_S = 5;
}

TEST_CASE("interchange first-route-node no-feeders",
          "[data_departure_interchange]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule2/motis");

  distributions_container::precomputed_distributions_container dummy(0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route edge of train RE_K_F from Kassel to Frankfurt
  auto const arriving_route_edge = graph_accessor::get_departing_route_edge(
      *schedule->route_index_to_first_route_node[schedule2::RE_K_F]);
  auto const& arriving_light_conn =
      arriving_route_edge->_m._route_edge._conns[0];

  REQUIRE(arriving_route_edge->_from->_station_node->_id == schedule2::KASSEL);
  REQUIRE(arriving_route_edge->_to->_station_node->_id == schedule2::FRANKFURT);
  REQUIRE(arriving_light_conn.d_time == 8 * 60);
  REQUIRE(arriving_light_conn.a_time == 10 * 60);

  // route node at Frankfurt of train ICE_F_S
  auto& route_node =
      *schedule->route_index_to_first_route_node[schedule2::ICE_F_S];
  // route edge from Frankfurt to Stuttgart
  auto const departing_route_edge =
      graph_accessor::get_departing_route_edge(route_node);
  auto const& departing_light_conn =
      departing_route_edge->_m._route_edge._conns[0];

  REQUIRE(route_node._station_node->_id == schedule2::FRANKFURT);
  REQUIRE(departing_route_edge->_from->_station_node->_id ==
          schedule2::FRANKFURT);
  REQUIRE(departing_route_edge->_to->_station_node->_id ==
          schedule2::STUTTGART);
  REQUIRE(departing_light_conn.d_time == 10 * 60 + 10);
  REQUIRE(departing_light_conn.a_time == 11 * 60 + 10);

  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  data_departure_interchange data(
      true, route_node, departing_light_conn, arriving_light_conn,
      dummy_arrival_distribution, *schedule, dummy, s_t_distributions);

  std::cout << "data.interchange_feeder_info_.waiting_time_ :"
            << data.interchange_feeder_info_.waiting_time_ << std::endl;

  REQUIRE(data.is_first_route_node_);
  REQUIRE(data.scheduled_departure_time_ == departing_light_conn.d_time);
  REQUIRE(data.largest_delay() == 1);
  REQUIRE(data.maximum_waiting_time_ == 0);
  REQUIRE(data.feeders_.size() == 0);
  REQUIRE(data.train_info_.first_departure_distribution_ ==
          &s_t_distributions.start_distribution_);

  REQUIRE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
          arriving_light_conn.a_time);
  REQUIRE(data.interchange_feeder_info_.arrival_distribution_ ==
          &dummy_arrival_distribution);
  REQUIRE(
      data.interchange_feeder_info_.transfer_time_ ==
      schedule->stations[route_node._station_node->_id]->get_transfer_time());
  REQUIRE(data.interchange_feeder_info_.waiting_time_ == 0);
}

TEST_CASE("interchange preceding-arrival no-feeders",
          "[data_departure_interchange]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule2/motis");

  precomputed_distributions_test_container precomputed({0.9, 0.1}, 0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route edge of train RE_K_F from Kassel to Frankfurt
  auto const arriving_route_edge = graph_accessor::get_departing_route_edge(
      *schedule->route_index_to_first_route_node[schedule2::RE_K_F]);
  auto const& arriving_light_conn =
      arriving_route_edge->_m._route_edge._conns[0];

  REQUIRE(arriving_route_edge->_from->_station_node->_id == schedule2::KASSEL);
  REQUIRE(arriving_route_edge->_to->_station_node->_id == schedule2::FRANKFURT);
  REQUIRE(arriving_light_conn.d_time == 8 * 60);
  REQUIRE(arriving_light_conn.a_time == 10 * 60);

  // light conn of route edge from Kassel to Frankfurt of train ICE_K_F_S
  auto const& preceding_arrival_light_conn =
      graph_accessor::get_departing_route_edge(
          *schedule->route_index_to_first_route_node[schedule2::ICE_K_F_S])
          ->_m._route_edge._conns[0];

  REQUIRE(preceding_arrival_light_conn.d_time == 9 * 60 + 15);
  REQUIRE(preceding_arrival_light_conn.a_time == 10 * 60 + 15);

  // route node at Frankfurt of train ICE_K_F_S
  auto& route_node =
      *graph_accessor::get_departing_route_edge(
           *schedule->route_index_to_first_route_node[schedule2::ICE_K_F_S])
           ->_to;

  REQUIRE(route_node._station_node->_id == schedule2::FRANKFURT);

  // route edge from Frankfurt to Stuttgart
  auto const departing_route_edge =
      graph_accessor::get_departing_route_edge(route_node);
  auto const& departing_light_conn =
      departing_route_edge->_m._route_edge._conns[0];

  REQUIRE(departing_route_edge->_from->_station_node->_id ==
          schedule2::FRANKFURT);
  REQUIRE(departing_route_edge->_to->_station_node->_id ==
          schedule2::STUTTGART);
  REQUIRE(departing_light_conn.d_time == 10 * 60 + 20);
  REQUIRE(departing_light_conn.a_time == 11 * 60 + 15);

  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  data_departure_interchange data(
      false, route_node, departing_light_conn, arriving_light_conn,
      dummy_arrival_distribution, *schedule, precomputed, s_t_distributions);

  REQUIRE_FALSE(data.is_first_route_node_);
  REQUIRE(data.scheduled_departure_time_ == departing_light_conn.d_time);
  REQUIRE(data.largest_delay() == 0);
  REQUIRE(data.maximum_waiting_time_ == 0);
  REQUIRE(data.feeders_.size() == 0);
  REQUIRE(data.train_info_.preceding_arrival_info_.arrival_time_ ==
          preceding_arrival_light_conn.a_time);
  REQUIRE(data.train_info_.preceding_arrival_info_.min_standing_ == 2);
  REQUIRE(data.train_info_.preceding_arrival_info_.arrival_distribution_ ==
          &precomputed.dist);

  REQUIRE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
          arriving_light_conn.a_time);
  REQUIRE(data.interchange_feeder_info_.arrival_distribution_ ==
          &dummy_arrival_distribution);
  REQUIRE(
      data.interchange_feeder_info_.transfer_time_ ==
      schedule->stations[route_node._station_node->_id]->get_transfer_time());
  REQUIRE(data.interchange_feeder_info_.waiting_time_ == 0);
}

TEST_CASE("interchange first-route-node feeders excl ic",
          "[data_departure_interchange]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule2/motis");

  precomputed_distributions_test_container precomputed({0.9, 0.1}, 0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route edge of train ICE_F_S from Frankfurt to Stuttgart
  auto const& arriving_route_edge =
      *graph_accessor::get_departing_route_edge(
          *schedule->route_index_to_first_route_node[schedule2::ICE_F_S]);
  auto const& arriving_light_conn =
      arriving_route_edge._m._route_edge._conns[0];

  REQUIRE(arriving_route_edge._from->_station_node->_id ==
          schedule2::FRANKFURT);
  REQUIRE(arriving_route_edge._to->_station_node->_id == schedule2::STUTTGART);
  REQUIRE(arriving_light_conn.d_time == 10 * 60 + 10);
  REQUIRE(arriving_light_conn.a_time == 11 * 60 + 10);

  // route node at Stuttgart of train S_S_E
  auto const& route_node =
      *schedule->route_index_to_first_route_node[schedule2::S_S_E];

  REQUIRE(route_node._station_node->_id == schedule2::STUTTGART);

  // route edge from Stuttgart to Erlangen of train S_S_E
  auto const departing_route_edge =
      graph_accessor::get_departing_route_edge(route_node);
  auto const& departing_light_conn =
      departing_route_edge->_m._route_edge._conns[0];

  REQUIRE(departing_route_edge->_from->_station_node->_id ==
          schedule2::STUTTGART);
  REQUIRE(departing_route_edge->_to->_station_node->_id == schedule2::ERLANGEN);
  REQUIRE(departing_light_conn.d_time == 11 * 60 + 30);
  REQUIRE(departing_light_conn.a_time == 15 * 60 + 30);

  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  data_departure_interchange data(
      true, route_node, departing_light_conn, arriving_light_conn,
      dummy_arrival_distribution, *schedule, precomputed, s_t_distributions);

  auto vec = graph_accessor::get_all_potential_feeders(route_node,
                                                       departing_light_conn);

  std::cout
      << schedule
             ->category_names[departing_light_conn._full_con->con_info->family]
      << departing_light_conn._full_con->con_info->train_nr << " "
      << departing_light_conn.d_time << " --> " << departing_light_conn.a_time
      << std::endl;
  for (auto const& v : vec) {
    std::cout
        << schedule->category_names[v->light_conn_._full_con->con_info->family]
        << v->light_conn_._full_con->con_info->train_nr << " "
        << v->light_conn_.d_time << " --> " << v->light_conn_.a_time
        << " waiting: " << graph_accessor::get_waiting_time(
                               schedule->waiting_time_rules_, v->light_conn_,
                               departing_light_conn) << std::endl;
  }

  // TODO: S Bahnen warten nicht aufeinander! nimm die beiden ICEs. S_F_S wird
  // wahrscheinlich gar nicht mehr benötigt.

  REQUIRE(data.is_first_route_node_);
  REQUIRE(data.scheduled_departure_time_ == departing_light_conn.d_time);
  REQUIRE(data.largest_delay() == 1);
  REQUIRE(data.feeders_.size() == 1);

  // Feeder S_F_S
  auto const& feeder_light_conn =
      graph_accessor::get_departing_route_edge(
          *schedule->route_index_to_first_route_node[schedule2::S_F_S])
          ->_m._route_edge._conns[0];
  duration const waiting_time = graph_accessor::get_waiting_time(
      schedule->waiting_time_rules_, feeder_light_conn, departing_light_conn);

  REQUIRE(data.feeders_[0].arrival_time_ == 11 * 60 + 15);
  REQUIRE(&data.feeders_[0].distribution_ == &precomputed.dist);
  REQUIRE(
      data.feeders_[0].transfer_time_ ==
      schedule->stations[route_node._station_node->_id]->get_transfer_time());
  REQUIRE(data.feeders_[0].latest_feasible_arrival_ ==
          (departing_light_conn.d_time - data.feeders_[0].transfer_time_) +
              waiting_time);

  REQUIRE(data.maximum_waiting_time_ == waiting_time);
  REQUIRE(data.interchange_feeder_info_.scheduled_arrival_time_ ==
          arriving_light_conn.a_time);
  REQUIRE(data.interchange_feeder_info_.arrival_distribution_ ==
          &dummy_arrival_distribution);
  REQUIRE(
      data.interchange_feeder_info_.transfer_time_ ==
      schedule->stations[route_node._station_node->_id]->get_transfer_time());
  REQUIRE(data.interchange_feeder_info_.waiting_time_ == 0);
}

TEST_CASE("interchange first-route-node feeders incl ic",
          "[data_departure_interchange]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule2/motis");
}
