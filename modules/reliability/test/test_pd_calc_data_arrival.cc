#include "catch/catch.hpp"

#include "motis/loader/loader.h"

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/probability_distribution.h"
#include "motis/reliability/pd_calc_data_arrival.h"
#include "motis/reliability/train_distributions.h"

#include "include/tt_distributions_test_manager.h"
#include "include/train_distributions_test_container.h"

using namespace motis;
using namespace motis::reliability;

TEST_CASE("initialize", "[pd_calc_data_arrival]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");

  // container delivering the departure distribution 0.8, 0.2
  train_distributions_test_container train_distributions({0.8, 0.2}, 0);
  tt_distributions_test_manager tt_distributions({0.1, 0.7, 0.2}, -1, 1);

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node = *schedule->route_index_to_first_route_node[4];
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& light_connection = first_route_edge->_m._route_edge._conns[0];
  auto const& second_route_node = *first_route_edge->_to;

  pd_calc_data_arrival data(second_route_node, light_connection, *schedule,
                            train_distributions, tt_distributions);

  REQUIRE(&data.route_node_ == &second_route_node);
  REQUIRE(&data.light_connection_ == &light_connection);
  REQUIRE(data.route_node_._station_node->_id == 1);
  REQUIRE(data.light_connection_.d_time == 5 * 60 + 55);
  REQUIRE(data.light_connection_.a_time == 6 * 60 + 5);

  REQUIRE(data.departure_info_.scheduled_departure_time_ ==
          data.light_connection_.d_time);
  REQUIRE(data.departure_info_.distribution_->first_minute() == 0);
  REQUIRE(equal(data.departure_info_.distribution_->probability_equal(0), 0.8));
  REQUIRE(equal(data.departure_info_.distribution_->probability_equal(1), 0.2));

  REQUIRE(data.travel_time_info_ ==
          tt_distributions.travel_distribution_.get());

  for (unsigned int dep_delay = 0; dep_delay <= 1; dep_delay++) {
    auto const& distribution =
        data.travel_time_info_->get_travel_time_distribution(dep_delay);
    REQUIRE(distribution.first_minute() == -1);
    REQUIRE(distribution.last_minute() == 1);
    REQUIRE(distribution.last_minute() == 1);
    REQUIRE(equal(distribution.probability_equal(-1), 0.1));
    REQUIRE(equal(distribution.probability_equal(0), 0.7));
    REQUIRE(equal(distribution.probability_equal(1), 0.2));
  }
}

TEST_CASE("test train_distributions", "[pd_calc_data_arrival]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");

  struct train_distributions_test2_container : train_distributions_container {
    train_distributions_test2_container() : train_distributions_container(0) {}
    probability_distribution const& get_probability_distribution(
        unsigned int const route_node_idx, unsigned int const light_conn_idx,
        type const t) const override {
      if (route_node_idx == 18 && light_conn_idx == 1 && t == departure)
        return train;
      return fail;
    }
    probability_distribution train;
    probability_distribution fail;
  } train_distributions;

  tt_distributions_test_manager tt_distributions({0.1, 0.7, 0.2}, -1, 1);

  // route node at Frankfurt of train ICE_FR_DA_H
  auto departure_route_node = *schedule->route_index_to_first_route_node[4];
  // route edge from Frankfurt to Darmstadt
  auto const route_edge =
      graph_accessor::get_departing_route_edge(departure_route_node);
  // get the second light connection
  auto const& light_connection = route_edge->_m._route_edge._conns[1];
  // route node at Darmstadt
  auto const& arrival_route_node = *route_edge->_to;

  pd_calc_data_arrival data(arrival_route_node, light_connection, *schedule,
                            train_distributions, tt_distributions);

  REQUIRE(departure_route_node._id == 18);
  REQUIRE(data.departure_info_.distribution_ == &train_distributions.train);
}

TEST_CASE("test tt_distributions", "[pd_calc_data_arrival]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");

  struct tt_distributions_test2_manager : tt_distributions_manager {

    travel_distribution_info const& get_travel_time_distributions(
        std::string const& train_category,
        duration const travel_time) const override {
      if (train_category == "ICE" && travel_time == 10) return distribution;
      return fail;
    }
    travel_distribution_info distribution;
    travel_distribution_info fail;
  } tt_distributions;

  train_distributions_test_container train_distributions({0.8, 0.2}, 0);

  // route node at Frankfurt of train ICE_FR_DA_H
  auto departure_route_node = *schedule->route_index_to_first_route_node[4];
  // route edge from Frankfurt to Darmstadt
  auto const route_edge =
      graph_accessor::get_departing_route_edge(departure_route_node);
  // get the second light connection
  auto const& light_connection = route_edge->_m._route_edge._conns[1];
  // route node at Darmstadt
  auto const& arrival_route_node = *route_edge->_to;

  pd_calc_data_arrival data(arrival_route_node, light_connection, *schedule,
                            train_distributions, tt_distributions);

  REQUIRE(data.travel_time_info_ == &tt_distributions.distribution);
}
