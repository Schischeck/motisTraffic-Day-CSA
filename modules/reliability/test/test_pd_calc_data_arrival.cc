#include "catch/catch.hpp"

#include "motis/loader/loader.h"

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/distributions_container.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/probability_distribution.h"
#include "motis/reliability/pd_calc_data_arrival.h"

#include "include/start_and_travel_test_distributions.h"
#include "include/train_distributions_test_container.h"

using namespace motis;
using namespace motis::reliability;

TEST_CASE("initialize", "[pd_calc_data_arrival]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");

  probability_distribution dep_dist;
  dep_dist.init({0.8, 0.2}, 0);
  start_and_travel_test_distributions s_t_distributions({0.1, 0.7, 0.2}, -1);

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node = *schedule->route_index_to_first_route_node[6];
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& light_connection = first_route_edge->_m._route_edge._conns[0];
  auto const& second_route_node = *first_route_edge->_to;

  pd_calc_data_arrival data(second_route_node, light_connection, dep_dist,
                            *schedule, s_t_distributions);

  REQUIRE(&data.route_node_ == &second_route_node);
  REQUIRE(&data.light_connection_ == &light_connection);
  REQUIRE(data.route_node_._station_node->_id == 1);
  REQUIRE(data.light_connection_.d_time == 5 * 60 + 55);
  REQUIRE(data.light_connection_.a_time == 6 * 60 + 5);

  REQUIRE(data.departure_info_.scheduled_departure_time_ ==
          data.light_connection_.d_time);
  REQUIRE(&data.departure_info_.distribution_ == &dep_dist);

  REQUIRE(data.travel_distributions_.size() == 2);
  REQUIRE(&data.travel_distributions_[0].get() ==
          &s_t_distributions.travel_distribution_);
  REQUIRE(&data.travel_distributions_[1].get() ==
          &s_t_distributions.travel_distribution_);

  REQUIRE(data.left_bound_ == -1);
  REQUIRE(data.right_bound_ == 2);
}

TEST_CASE("test s_t_distributions", "[pd_calc_data_arrival]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");

  struct start_and_travel_test2_distributions : start_and_travel_distributions {
    start_and_travel_test2_distributions() {
      distribution_.init_one_point(0, 1.0);
    }
    probability_distribution const& get_start_distribution(
        std::string const& family) const override {
      return distribution_;
    }
    void get_travel_time_distributions(
        std::string const& family, unsigned int const travel_time,
        unsigned int const to_departure_delay,
        std::vector<probability_distribution_cref>& distributions)
        const override {
      if (family == "ICE" && travel_time == 10) {
        for (unsigned int d = 0; d <= to_departure_delay; d++) {
          distributions.push_back(std::cref(distribution_));
        }
      }
    }
    probability_distribution distribution_;
  } s_t_distributions;

  probability_distribution dep_dist;
  dep_dist.init({0.8, 0.2}, 0);

  // route node at Frankfurt of train ICE_FR_DA_H
  auto departure_route_node = *schedule->route_index_to_first_route_node[6];
  // route edge from Frankfurt to Darmstadt
  auto const route_edge =
      graph_accessor::get_departing_route_edge(departure_route_node);
  // get the second light connection
  auto const& light_connection = route_edge->_m._route_edge._conns[1];
  // route node at Darmstadt
  auto const& arrival_route_node = *route_edge->_to;

  pd_calc_data_arrival data(arrival_route_node, light_connection, dep_dist,
                            *schedule, s_t_distributions);

  REQUIRE(data.travel_distributions_.size() == 2);
  REQUIRE(&data.travel_distributions_[0].get() ==
          &s_t_distributions.distribution_);
  REQUIRE(&data.travel_distributions_[1].get() ==
          &s_t_distributions.distribution_);
}
