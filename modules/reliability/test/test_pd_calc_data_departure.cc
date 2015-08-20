#include "catch/catch.hpp"

#include "include/start_and_travel_test_distributions.h"
#include "motis/loader/loader.h"

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/pd_calc_data_departure.h"
#include "motis/reliability/pd_calc_data_arrival.h"
#include "motis/reliability/train_distributions.h"

#include "include/train_distributions_test_container.h"

using namespace motis;
using namespace motis::reliability;

TEST_CASE("first-route-node no-feeders", "[pd_calc_data_departure]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");

  train_distributions_container dummy(0);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node = *schedule->route_index_to_first_route_node[4];
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& first_light_conn = first_route_edge->_m._route_edge._conns[0];

  pd_calc_data_departure data(first_route_node, first_light_conn, true,
                              *schedule, dummy, s_t_distributions);

  REQUIRE(&data.route_node_ == &first_route_node);
  REQUIRE(data.route_node_._station_node->_id == 2);
  REQUIRE(data.light_connection_.d_time == 5 * 60 + 55);
  REQUIRE(data.light_connection_.a_time == 6 * 60 + 5);
  REQUIRE(data.scheduled_departure_time() == data.light_connection_.d_time);
  REQUIRE(data.largest_delay() == 1);
  REQUIRE(data.maximum_waiting_time_ == 0);
  REQUIRE(data.light_connection_._full_con->con_info->train_nr == 5);
  REQUIRE(data.is_first_route_node_);

  auto const& start_distribution =
      data.train_info_.first_departure_distribution;
  REQUIRE(equal(start_distribution->sum(), 1.0));
  REQUIRE(start_distribution->first_minute() == 0);
  REQUIRE(start_distribution->last_minute() == 1);
  REQUIRE(equal(start_distribution->probability_equal(0), 0.6));
  REQUIRE(equal(start_distribution->probability_equal(1), 0.4));
}

TEST_CASE("preceding-arrival no-feeders", "[pd_calc_data_departure]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");

  train_distributions_test_container train_distributions({0.1, 0.7, 0.2}, -1);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route node at Hanau of train ICE_HA_W_HE
  auto& first_route_node = *schedule->route_index_to_first_route_node[5];
  // route node at Wuerzburg
  auto second_route_node =
      graph_accessor::get_departing_route_edge(first_route_node)->_to;
  // route edge from Wuerzburg to Heilbronn
  auto const route_edge =
      graph_accessor::get_departing_route_edge(*second_route_node);
  auto const& light_connection = route_edge->_m._route_edge._conns[0];

  pd_calc_data_departure data(*second_route_node, light_connection, false,
                              *schedule, train_distributions,
                              s_t_distributions);

  REQUIRE(&data.route_node_ == second_route_node);
  REQUIRE(&data.light_connection_ == &light_connection);
  REQUIRE(data.route_node_._station_node->_id == 7);  // Wuerzburg
  REQUIRE(data.light_connection_.d_time == 10 * 60 + 34);
  REQUIRE(data.light_connection_.a_time == 11 * 60 + 7);
  REQUIRE(data.scheduled_departure_time() == data.light_connection_.d_time);
  REQUIRE(data.largest_delay() == 1);
  REQUIRE(!data.is_first_route_node_);
  REQUIRE(data.maximum_waiting_time_ == 0);
  REQUIRE(data.feeders_.size() == 0);

  REQUIRE(data.train_info_.preceding_arrival_info_.arrival_time_ ==
          10 * 60 + 32);
  REQUIRE(data.train_info_.preceding_arrival_info_.min_standing_ == 2);
  REQUIRE(data.train_info_.preceding_arrival_info_.arrival_distribution_ ==
          &train_distributions.get_probability_distribution(
              0, 0, train_distributions_container::type::arrival));
}

TEST_CASE("first-route-node feeders", "[pd_calc_data_departure]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");

  train_distributions_test_container train_distributions({0.1, 0.7, 0.2}, -1);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route node at Darmstadt of train IC_DA_H
  auto& first_route_node = *schedule->route_index_to_first_route_node[0];
  // route edge from Darmstadt to Heidelberg
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  // light connection d07:00 a07:28
  auto const& light_connection = first_route_edge->_m._route_edge._conns[1];

  pd_calc_data_departure data(first_route_node, light_connection, true,
                              *schedule, train_distributions,
                              s_t_distributions);

  REQUIRE(&data.route_node_ == &first_route_node);
  REQUIRE(&data.light_connection_ == &light_connection);
  REQUIRE(data.route_node_._station_node->_id == 1);  // Darmstadt
  REQUIRE(data.light_connection_.d_time == 7 * 60);
  REQUIRE(data.light_connection_.a_time == 7 * 60 + 28);
  REQUIRE(data.scheduled_departure_time() == data.light_connection_.d_time);
  REQUIRE(data.largest_delay() == data.maximum_waiting_time_);
  REQUIRE(data.is_first_route_node_);

  REQUIRE(data.train_info_.first_departure_distribution ==
          &s_t_distributions.get_start_distribution("dummy"));

  REQUIRE(data.maximum_waiting_time_ == 3);
  REQUIRE(data.feeders_.size() == 2);

  REQUIRE(data.feeders_[0].arrival_time_ == 6 * 60 + 54);
  REQUIRE(data.feeders_[0].transfer_time_ ==
          5);  // TODO use platform change time
  REQUIRE(data.feeders_[0].latest_feasible_arrival_ ==
          (7 * 60 + 3) - 5);  // TODO use platform change time
  REQUIRE(&data.feeders_[0].distribution_ ==
          &train_distributions.get_probability_distribution(
              0, 0, train_distributions_container::type::arrival));

  REQUIRE(data.feeders_[1].arrival_time_ == 6 * 60 + 41);
  REQUIRE(data.feeders_[1].transfer_time_ == 5);
  REQUIRE(data.feeders_[1].latest_feasible_arrival_ == (7 * 60 + 3) - 5);
  REQUIRE(&data.feeders_[1].distribution_ ==
          &train_distributions.get_probability_distribution(
              0, 0, train_distributions_container::type::arrival));
}

TEST_CASE("preceding-arrival feeders", "[pd_calc_data_departure]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");

  train_distributions_test_container train_distributions({0.1, 0.7, 0.2}, -1);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route node at Darmstadt of train ICE_FR_DA_H
  auto& route_node = *graph_accessor::get_departing_route_edge(
                          *schedule->route_index_to_first_route_node[4])->_to;
  auto const& light_connection = graph_accessor::get_departing_route_edge(
                                     route_node)->_m._route_edge._conns[0];

  pd_calc_data_departure data(route_node, light_connection, false, *schedule,
                              train_distributions, s_t_distributions);

  REQUIRE(&data.route_node_ == &route_node);
  REQUIRE(&data.light_connection_ == &light_connection);
  REQUIRE(data.route_node_._station_node->_id == 1);  // Darmstadt
  REQUIRE(data.light_connection_.d_time == 6 * 60 + 11);
  REQUIRE(data.light_connection_.a_time == 6 * 60 + 45);
  REQUIRE(data.scheduled_departure_time() == data.light_connection_.d_time);
  REQUIRE(data.largest_delay() == data.maximum_waiting_time_);
  REQUIRE(!data.is_first_route_node_);

  REQUIRE(data.train_info_.preceding_arrival_info_.arrival_time_ == 6 * 60 + 5);
  REQUIRE(data.train_info_.preceding_arrival_info_.min_standing_ == 2);
  REQUIRE(data.train_info_.preceding_arrival_info_.arrival_distribution_ ==
          &train_distributions.get_probability_distribution(
              0, 0, train_distributions_container::type::arrival));

  REQUIRE(data.maximum_waiting_time_ == 3);
  REQUIRE(data.feeders_.size() == 2);

  REQUIRE(data.feeders_[0].arrival_time_ == 5 * 60 + 41);
  REQUIRE(data.feeders_[0].transfer_time_ == 5);
  REQUIRE(data.feeders_[0].latest_feasible_arrival_ == (6 * 60 + 11 + 3) - 5);
  REQUIRE(&data.feeders_[0].distribution_ ==
          &train_distributions.get_probability_distribution(
              0, 0, train_distributions_container::type::arrival));

  REQUIRE(data.feeders_[1].arrival_time_ == 5 * 60 + 56);
  REQUIRE(data.feeders_[1].transfer_time_ == 5);
  REQUIRE(data.feeders_[1].latest_feasible_arrival_ == (6 * 60 + 11 + 3) - 5);
  REQUIRE(&data.feeders_[1].distribution_ ==
          &train_distributions.get_probability_distribution(
              0, 0, train_distributions_container::type::arrival));
}

TEST_CASE("check train_distributions", "[pd_calc_data_departure]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");

  struct train_distributions_test2_container : train_distributions_container {
    train_distributions_test2_container() : train_distributions_container(0) {}
    probability_distribution const& get_probability_distribution(
        unsigned int const route_node_idx, unsigned int const light_conn_idx,
        type const t) const override {
      if (route_node_idx == 19 && light_conn_idx == 0 && t == arrival)
        return train;
      if (route_node_idx == 15 && light_conn_idx == 0 && t == arrival)
        return feeder1;
      if (route_node_idx == 15 && light_conn_idx == 1 && t == arrival)
        return feeder2;
      return fail;
    }
    probability_distribution train;
    probability_distribution feeder1;
    probability_distribution feeder2;
    probability_distribution fail;
  } train_distributions;
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route node at Darmstadt of train ICE_FR_DA_H
  auto& route_node = *graph_accessor::get_departing_route_edge(
                          *schedule->route_index_to_first_route_node[4])->_to;
  auto const& light_connection = graph_accessor::get_departing_route_edge(
                                     route_node)->_m._route_edge._conns[0];

  pd_calc_data_departure data(route_node, light_connection, false, *schedule,
                              train_distributions, s_t_distributions);

  // route node at Darmstadt of train IC_FH_DA
  REQUIRE(graph_accessor::get_departing_route_edge(
              *schedule->route_index_to_first_route_node[2])->_to->_id == 15);
  REQUIRE(data.route_node_._id == 19);
  REQUIRE(data.train_info_.preceding_arrival_info_.arrival_distribution_ ==
          &train_distributions.train);
  REQUIRE(&data.feeders_[0].distribution_ == &train_distributions.feeder1);
  REQUIRE(&data.feeders_[1].distribution_ == &train_distributions.feeder2);
}

TEST_CASE("check start distribution", "[pd_calc_data_departure]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");

  train_distributions_container dummy(0);
  struct start_and_travel_test2_distributions : start_and_travel_distributions {
    probability_distribution const& get_start_distribution(
        std::string const& train_category) const override {
      if (train_category == "ICE") return distribution;
      return fail;
    }
    void get_travel_time_distributions(
        std::string const& family, unsigned int const travel_time,
        std::vector<travel_time_distribution>& distributions) const override {}
    probability_distribution distribution;
    probability_distribution fail;
  } s_t_distributions;

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node = *schedule->route_index_to_first_route_node[4];
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& first_light_conn = first_route_edge->_m._route_edge._conns[0];

  pd_calc_data_departure data(first_route_node, first_light_conn, true,
                              *schedule, dummy, s_t_distributions);

  REQUIRE(data.train_info_.first_departure_distribution ==
          &s_t_distributions.distribution);
}

/* In this test case, largest delay depends on the preceding arrival.
 * All other cases in largest_delay() have been tested
 * in the other test cases */
TEST_CASE("check largest delay", "[pd_calc_data_departure]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");

  train_distributions_test_container train_distributions(
      {0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1}, -1);
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  // route node at Darmstadt of train ICE_FR_DA_H
  auto& route_node = *graph_accessor::get_departing_route_edge(
                          *schedule->route_index_to_first_route_node[4])->_to;
  auto const& light_connection = graph_accessor::get_departing_route_edge(
                                     route_node)->_m._route_edge._conns[0];

  pd_calc_data_departure data(route_node, light_connection, false, *schedule,
                              train_distributions, s_t_distributions);

  REQUIRE(data.maximum_waiting_time_ == 3);
  REQUIRE(data.largest_delay() == 4);
}
