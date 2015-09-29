
#define CATCH_CONFIG_MAIN

#include "catch/catch.hpp"

#include "motis/loader/loader.h"

#include "motis/core/common/date_util.h"

#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/schedule.h"

#include "motis/reliability/distributions_calculator.h"
#include "motis/reliability/distributions_container.h"
#include "motis/reliability/graph_accessor.h"

#include "include/start_and_travel_test_distributions.h"

using namespace motis;
using namespace motis::reliability;

namespace schedule1 {
/* train numbers */
short const IC_DA_H = 1;
short const IC_FR_DA = 2;
short const IC_FH_DA = 3;
short const RE_MA_DA = 4;
short const ICE_FR_DA_H = 5;
short const ICE_HA_W_HE = 6;
short const ICE_K_K = 7;
short const RE_K_S = 8;
}

TEST_CASE("is_pre_computed_train", "[distributions_calculator]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));

  REQUIRE(
      distributions_calculator::precomputation::detail::is_pre_computed_route(
          *schedule, *graph_accessor::get_first_route_node(
                         *schedule, schedule1::IC_DA_H)));
  REQUIRE(
      distributions_calculator::precomputation::detail::is_pre_computed_route(
          *schedule, *graph_accessor::get_first_route_node(
                         *schedule, schedule1::ICE_FR_DA_H)));
  REQUIRE_FALSE(
      distributions_calculator::precomputation::detail::is_pre_computed_route(
          *schedule,
          *graph_accessor::get_first_route_node(*schedule, schedule1::RE_K_S)));
}

void test_distributions(
    node const& route_node,
    distributions_container::precomputed_distributions_container&
        precomputed_distributions,
    bool const pre_computed_distributions) {
  auto const route_edge = graph_accessor::get_departing_route_edge(route_node);
  // last route node
  if (route_edge == nullptr) {
    REQUIRE_FALSE(precomputed_distributions.contains_distributions(
        route_node._id, distributions_container::departure));
    return;
  }
  auto const& head_route_node = *route_edge->_to;
  if (pre_computed_distributions) {
    REQUIRE(precomputed_distributions.contains_distributions(
        route_node._id, distributions_container::departure));
    // first route node
    if (graph_accessor::get_arriving_route_edge(route_node) == nullptr) {
      REQUIRE_FALSE(precomputed_distributions.contains_distributions(
          route_node._id, distributions_container::arrival));
    } else {
      REQUIRE(precomputed_distributions.contains_distributions(
          route_node._id, distributions_container::arrival));
    }

    for (unsigned int l = 0; l < route_edge->_m._route_edge._conns.size();
         l++) {
      auto const& departure_distribution =
          precomputed_distributions.get_distribution(
              route_node._id, l, distributions_container::departure);
      auto const& arrival_distribution =
          precomputed_distributions.get_distribution(
              head_route_node._id, l, distributions_container::arrival);
      REQUIRE_FALSE(departure_distribution.empty());
      REQUIRE(equal(departure_distribution.sum(), 1.0));
      REQUIRE_FALSE(arrival_distribution.empty());
      REQUIRE(equal(arrival_distribution.sum(), 1.0));
    }
  } else {
    REQUIRE_FALSE(precomputed_distributions.contains_distributions(
        route_node._id, distributions_container::departure));
    REQUIRE_FALSE(precomputed_distributions.contains_distributions(
        head_route_node._id, distributions_container::arrival));
  }

  test_distributions(head_route_node, precomputed_distributions,
                     pre_computed_distributions);
}

TEST_CASE("Initial_distributions_simple", "[distributions_calculator]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));
  distributions_container::precomputed_distributions_container
      precomputed_distributions(schedule->node_count);
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

#if 0
#include "motis/reliability/db_distributions.h"
TEST_CASE("Initial_distributions_db_distributions",
          "[distributions_calculator]") {
  std::cout << "Initial_distributions_db_distributions" << std::endl;
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));
  distributions_container::precomputed_distributions_container
      precomputed_distributions(schedule->node_count);
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
TEST_CASE("Initial_distributions_db_distributions2",
          "[distributions_calculator]") {
  std::cout << "Initial_distributions_db_distributions2" << std::endl;
  auto schedule = loader::load_schedule("/tmp/rohdaten/rohdaten/",
                                        to_unix_time(2015, 9, 28),
                                        to_unix_time(2015, 9, 29));
  std::cout << "schedule loaded" << std::endl;
  distributions_container::precomputed_distributions_container
      precomputed_distributions(schedule->node_count);
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
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));
  distributions_container::precomputed_distributions_container
      precomputed_distributions(schedule->node_count);
  start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                        {0.1, 0.8, 0.1}, -1);
  distributions_calculator::precomputation::perform_precomputation(
      *schedule, s_t_distributions, precomputed_distributions);
  distributions_container::ride_distributions_container container;

  // route node at Karlsruhe of train RE_K_S
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule, schedule1::RE_K_S);
  node const& second_route_node =
      *graph_accessor::get_departing_route_edge(first_route_node)->_to;

  distributions_calculator::ride_distribution::compute_distributions_for_a_ride(
      first_route_node, 0, second_route_node, *schedule, s_t_distributions,
      precomputed_distributions, container);

  REQUIRE_FALSE(
      container.get_distribution(first_route_node._id, 0,
                                 distributions_container::departure).empty());
  REQUIRE(equal(
      container.get_distribution(first_route_node._id, 0,
                                 distributions_container::departure).sum(),
      1.0));

  REQUIRE_FALSE(
      container.get_distribution(second_route_node._id, 0,
                                 distributions_container::arrival).empty());
  REQUIRE(
      equal(container.get_distribution(second_route_node._id, 0,
                                       distributions_container::arrival).sum(),
            1.0));
}

TEST_CASE("distributions for a ride ICE", "[distributions_calculator]") {
  auto schedule = loader::load_schedule(
      "../modules/reliability/resources/schedule/", to_unix_time(2015, 9, 28),
      to_unix_time(2015, 9, 29));
  distributions_container::precomputed_distributions_container
      precomputed_distributions(schedule->node_count);
  start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                        {0.1, 0.8, 0.1}, -1);
  distributions_calculator::precomputation::perform_precomputation(
      *schedule, s_t_distributions, precomputed_distributions);
  distributions_container::ride_distributions_container container;

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule, schedule1::ICE_FR_DA_H);
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
                                   distributions_container::departure);
    REQUIRE_FALSE(distribution.empty());
    REQUIRE(equal(distribution.sum(), 1.0));
  }
  {
    auto const& distribution =
        container.get_distribution(second_route_node._id, light_conn_idx,
                                   distributions_container::arrival);
    REQUIRE_FALSE(distribution.empty());
    REQUIRE(equal(distribution.sum(), 1.0));
  }
  {
    auto const& distribution =
        container.get_distribution(second_route_node._id, light_conn_idx,
                                   distributions_container::departure);
    REQUIRE_FALSE(distribution.empty());
    REQUIRE(equal(distribution.sum(), 1.0));
  }
  {
    auto const& distribution = container.get_distribution(
        last_route_node._id, light_conn_idx, distributions_container::arrival);
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
