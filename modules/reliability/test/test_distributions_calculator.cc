
#define CATCH_CONFIG_MAIN

#include "catch/catch.hpp"

#include "motis/loader/loader.h"

#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/schedule.h"

#include "motis/reliability/distributions_calculator.h"
#include "motis/reliability/distributions_container.h"
#include "motis/reliability/graph_accessor.h"

#include "include/start_and_travel_test_distributions.h"

using namespace motis;
using namespace motis::reliability;

TEST_CASE("is_pre_computed_train", "[distributions_calculator]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");

  REQUIRE(
      distributions_calculator::precomputation::detail::is_pre_computed_route(
          *schedule,
          *schedule->route_index_to_first_route_node[0]));  // IC_DA_H

  REQUIRE(
      distributions_calculator::precomputation::detail::is_pre_computed_route(
          *schedule,

          *schedule->route_index_to_first_route_node[6]));  // ICE_FR_DA_H

  // RE_K_S
  REQUIRE_FALSE(
      distributions_calculator::precomputation::detail::is_pre_computed_route(
          *schedule, *schedule->route_index_to_first_route_node[5]));
}

void test_distributions(
    node const& route_node,
    precomputed_distributions_container& precomputed_distributions,
    bool const pre_computed_distributions) {
  auto const route_edge = graph_accessor::get_departing_route_edge(route_node);
  // last route node
  if (route_edge == nullptr) {
    REQUIRE_FALSE(precomputed_distributions.contains_departure_distributions(
        route_node._id));
    return;
  }
  auto const& head_route_node = *route_edge->_to;
  if (pre_computed_distributions) {
    REQUIRE(precomputed_distributions.contains_departure_distributions(
        route_node._id));
    // first route node
    if (graph_accessor::get_arriving_route_edge(route_node) == nullptr) {
      REQUIRE_FALSE(precomputed_distributions.contains_arrival_distributions(
          route_node._id));
    } else {
      REQUIRE(precomputed_distributions.contains_arrival_distributions(
          route_node._id));
    }

    auto const& departure_distributions =
        precomputed_distributions.get_route_node_distributions(
            route_node._id, precomputed_distributions_container::departure);
    auto const& arrival_distributions =
        precomputed_distributions.get_route_node_distributions(
            head_route_node._id, precomputed_distributions_container::arrival);

    for (unsigned int l = 0; l < route_edge->_m._route_edge._conns.size();
         l++) {
      REQUIRE_FALSE(departure_distributions.get_distribution(l).empty());
      REQUIRE(equal(departure_distributions.get_distribution(l).sum(), 1.0));
      REQUIRE_FALSE(arrival_distributions.get_distribution(l).empty());
      REQUIRE(equal(arrival_distributions.get_distribution(l).sum(), 1.0));
    }
  } else {
    REQUIRE_FALSE(precomputed_distributions.contains_departure_distributions(
        route_node._id));
    REQUIRE_FALSE(precomputed_distributions.contains_arrival_distributions(
        head_route_node._id));
  }

  test_distributions(head_route_node, precomputed_distributions,
                     pre_computed_distributions);
}

TEST_CASE("Initial_distributions_simple", "[distributions_calculator]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");
  precomputed_distributions_container precomputed_distributions(
      schedule->node_count);
  start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                        {0.1, 0.8, 0.1}, -1);

  distributions_calculator::precomputation::perform_precomputation(
      *schedule, s_t_distributions, precomputed_distributions);

  for (auto const first_route_node :
       schedule->route_index_to_first_route_node) {
    test_distributions(
        *first_route_node, precomputed_distributions,
        distributions_calculator::precomputation::detail::is_pre_computed_route(
            *schedule, *first_route_node));
  }
}

#include "motis/reliability/db_distributions.h"
TEST_CASE("Initial_distributions_db_distributions",
          "[distributions_calculator]") {
  std::cout << "Initial_distributions_db_distributions" << std::endl;
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");
  precomputed_distributions_container precomputed_distributions(
      schedule->node_count);
  db_distributions db_dists(
      "/home/keyhani/git/motis/DBDists/DBData/20130805/Original/td/", 120,
      120);  // todo: read max travel time from graph

  distributions_calculator::precomputation::perform_precomputation(
      *schedule, db_dists, precomputed_distributions);

  for (auto const first_route_node :
       schedule->route_index_to_first_route_node) {
    test_distributions(
        *first_route_node, precomputed_distributions,
        distributions_calculator::precomputation::detail::is_pre_computed_route(
            *schedule, *first_route_node));
  }
}

#if 1
TEST_CASE("Initial_distributions_db_distributions2",
          "[distributions_calculator]") {
  std::cout << "Initial_distributions_db_distributions2" << std::endl;
  auto schedule = load_text_schedule(
      "/data/schedule/2015-08-26_motis_6904ee18-73a0-474e-840b-f49586f4baf1/"
      "schedule/motis");
  std::cout << "schedule loaded" << std::endl;
  precomputed_distributions_container precomputed_distributions(
      schedule->node_count);
  db_distributions db_dists(
      "/home/keyhani/git/motis/DBDists/DBData/20130805/Original/td/", 120,
      120);  // todo: read max travel time from graph

  distributions_calculator::precomputation::perform_precomputation(
      *schedule, db_dists, precomputed_distributions);

  for (auto const first_route_node :
       schedule->route_index_to_first_route_node) {
    test_distributions(
        *first_route_node, precomputed_distributions,
        distributions_calculator::precomputation::detail::is_pre_computed_route(
            *schedule, *first_route_node));
  }
}
#endif

TEST_CASE("distributions for a ride RE", "[distributions_calculator]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");
  precomputed_distributions_container precomputed_distributions(
      schedule->node_count);
  start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                        {0.1, 0.8, 0.1}, -1);
  distributions_calculator::precomputation::perform_precomputation(
      *schedule, s_t_distributions, precomputed_distributions);
  ride_distributions_container container;

  // route node at Karlsruhe of train RE_K_S
  auto& first_route_node = *schedule->route_index_to_first_route_node[5];
  node const& second_route_node =
      *graph_accessor::get_departing_route_edge(first_route_node)->_to;

  distributions_calculator::ride_distribution::compute_distributions_for_a_ride(
      first_route_node, 0, second_route_node, *schedule, s_t_distributions,
      precomputed_distributions, container);

  REQUIRE_FALSE(container.get_distribution(
                              first_route_node._id, 0,
                              ride_distributions_container::departure).empty());
  REQUIRE(equal(
      container.get_distribution(first_route_node._id, 0,
                                 ride_distributions_container::departure).sum(),
      1.0));

  REQUIRE_FALSE(container.get_distribution(
                              second_route_node._id, 0,
                              ride_distributions_container::arrival).empty());
  REQUIRE(equal(
      container.get_distribution(second_route_node._id, 0,
                                 ride_distributions_container::arrival).sum(),
      1.0));
}

TEST_CASE("distributions for a ride ICE", "[distributions_calculator]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");
  precomputed_distributions_container precomputed_distributions(
      schedule->node_count);
  start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                        {0.1, 0.8, 0.1}, -1);
  distributions_calculator::precomputation::perform_precomputation(
      *schedule, s_t_distributions, precomputed_distributions);
  ride_distributions_container container;

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node = *schedule->route_index_to_first_route_node[6];
  node const& second_route_node =
      *graph_accessor::get_departing_route_edge(first_route_node)->_to;
  node const& last_route_node =
      *graph_accessor::get_departing_route_edge(second_route_node)->_to;

  unsigned int const light_conn_idx = 1;

  distributions_calculator::ride_distribution::compute_distributions_for_a_ride(
      first_route_node, light_conn_idx, last_route_node, *schedule,
      s_t_distributions, precomputed_distributions, container);

  {
    auto const& distribution =
        container.get_distribution(first_route_node._id, light_conn_idx,
                                   ride_distributions_container::departure);
    REQUIRE_FALSE(distribution.empty());
    REQUIRE(equal(distribution.sum(), 1.0));
  }
  {
    auto const& distribution =
        container.get_distribution(second_route_node._id, light_conn_idx,
                                   ride_distributions_container::arrival);
    REQUIRE_FALSE(distribution.empty());
    REQUIRE(equal(distribution.sum(), 1.0));
  }
  {
    auto const& distribution =
        container.get_distribution(second_route_node._id, light_conn_idx,
                                   ride_distributions_container::departure);
    REQUIRE_FALSE(distribution.empty());
    REQUIRE(equal(distribution.sum(), 1.0));
  }
  {
    auto const& distribution =
        container.get_distribution(last_route_node._id, light_conn_idx,
                                   ride_distributions_container::arrival);
    REQUIRE_FALSE(distribution.empty());
    REQUIRE(equal(distribution.sum(), 1.0));
  }
}

TEST_CASE("Test queue element", "[distributions_calculator]") {
  distributions_calculator::common::queue_type queue;

  node dummy_node(nullptr, 0);

  light_connection lc1(1);
  light_connection lc2(2);
  light_connection lc3(3);

  queue.emplace(&dummy_node, &dummy_node, &lc3, 0, false);
  queue.emplace(&dummy_node, &dummy_node, &lc2, 0, false);
  queue.emplace(&dummy_node, &dummy_node, &lc1, 0, false);
  queue.emplace(&dummy_node, &dummy_node, &lc3, 0, false);

  REQUIRE(queue.top().light_connection_->d_time == 1);
  queue.pop();
  REQUIRE(queue.top().light_connection_->d_time == 2);
  queue.pop();
  REQUIRE(queue.top().light_connection_->d_time == 3);
  queue.pop();

  queue.emplace(&dummy_node, &dummy_node, &lc2, 0, false);

  REQUIRE(queue.top().light_connection_->d_time == 2);
  queue.pop();
  REQUIRE(queue.top().light_connection_->d_time == 3);
  queue.pop();
  REQUIRE(queue.empty());
}
