#include "catch/catch.hpp"

#include "motis/loader/loader.h"

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/pd_calc_data_departure.h"
#include "motis/reliability/pd_calc_data_arrival.h"
#include "motis/reliability/train_distributions.h"
#include "include/tt_distributions_test_manager.h"

using namespace motis;
using namespace motis::reliability;

TEST_CASE("first-route-node no-feeders", "[pd_calc_data_departure]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");

  train_distributions_container train_distributions(schedule->node_count);
  tt_distributions_test_manager tt_distributions;
  tt_distributions.initialize();

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node = *schedule->route_index_to_first_route_node[4];
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& first_light_conn = first_route_edge->_m._route_edge._conns[0];

  pd_calc_data_departure data(first_route_node, first_light_conn, true,
                              *schedule, train_distributions, tt_distributions);

  REQUIRE(data.route_node_._id == 15);
  REQUIRE(data.route_node_._station_node->_id == 2);
  REQUIRE(data.light_connection_.d_time == 5 * 60 + 55);
  REQUIRE(data.light_connection_.a_time == 6 * 60 + 5);
  REQUIRE(data.scheduled_departure_time() == 5*60+55);
  REQUIRE(data.largest_delay() == 1);
  REQUIRE(data.light_connection_._full_con->con_info->train_nr == 5);
  REQUIRE(data.is_first_route_node_);

  auto const& start_distribution = data.train_info_.first_departure_distribution;
  REQUIRE(equal(start_distribution->sum(), 1.0));
  REQUIRE(start_distribution->first_minute() == 0);
  REQUIRE(start_distribution->last_minute() == 1);
  REQUIRE(equal(start_distribution->probability_equal(0), 0.5));
  REQUIRE(equal(start_distribution->probability_equal(1), 0.5));
}

TEST_CASE("preceding-arrival no-feeders", "[pd_calc_data_departure]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");

  train_distributions_container train_distributions(schedule->node_count);
  tt_distributions_test_manager tt_distributions;
  tt_distributions.initialize();

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node = *schedule->route_index_to_first_route_node[4];
  // route node at Darmstadt
  auto second_route_node =
      graph_accessor::get_departing_route_edge(first_route_node)->_to;
  // route edge from Darmstadt to Heidelberg
  auto const route_edge =
        graph_accessor::get_departing_route_edge(*second_route_node);
  auto const& light_connection = route_edge->_m._route_edge._conns[0];

  /*pd_calc_data_departure data(*second_route_node, light_connection, false,
                              *schedule, train_distributions, tt_distributions);
  data.debug_output(std::cout);*/
}

TEST_CASE("first-route-node feeders", "[pd_calc_data_departure]") {

}

TEST_CASE("preceding-arrival feeders", "[pd_calc_data_departure]") {

}
