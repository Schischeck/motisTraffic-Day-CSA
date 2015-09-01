
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

TEST_CASE("is_pre_computed_train", "[train_dist_calc]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");

  REQUIRE(distributions_calculator::is_pre_computed_route(
      *schedule,
      *schedule->route_index_to_first_route_node[0]));  // IC_DA_H

  REQUIRE(distributions_calculator::is_pre_computed_route(
      *schedule,

      *schedule->route_index_to_first_route_node[6]));  // ICE_FR_DA_H

  // RE_K_S
  REQUIRE_FALSE(distributions_calculator::is_pre_computed_route(
      *schedule, *schedule->route_index_to_first_route_node[5]));
}

void test_distributions(
    node const& route_node,
    precomputed_distributions_container& train_distributions,
    bool const pre_computed_distributions) {
  auto const route_edge = graph_accessor::get_departing_route_edge(route_node);
  // last route node
  if (route_edge == nullptr) {
    REQUIRE_FALSE(
        train_distributions.contains_departure_distributions(route_node._id));
    return;
  }
  auto const& head_route_node = *route_edge->_to;
  if (pre_computed_distributions) {
    REQUIRE(
        train_distributions.contains_departure_distributions(route_node._id));
    // first route node
    if (graph_accessor::get_arriving_route_edge(route_node) == nullptr) {
      REQUIRE_FALSE(
          train_distributions.contains_arrival_distributions(route_node._id));
    } else {
      REQUIRE(
          train_distributions.contains_arrival_distributions(route_node._id));
    }

    auto const& departure_distributions =
        train_distributions.get_route_node_distributions(
            route_node._id, precomputed_distributions_container::departure);
    auto const& arrival_distributions =
        train_distributions.get_route_node_distributions(
            head_route_node._id, precomputed_distributions_container::arrival);

    for (unsigned int l = 0; l < route_edge->_m._route_edge._conns.size();
         l++) {
      REQUIRE_FALSE(departure_distributions.get_distribution(l).empty());
      REQUIRE(equal(departure_distributions.get_distribution(l).sum(), 1.0));
      REQUIRE_FALSE(arrival_distributions.get_distribution(l).empty());
      REQUIRE(equal(arrival_distributions.get_distribution(l).sum(), 1.0));
    }
  } else {
    REQUIRE_FALSE(
        train_distributions.contains_departure_distributions(route_node._id));
    REQUIRE_FALSE(train_distributions.contains_arrival_distributions(
        head_route_node._id));
  }

  test_distributions(head_route_node, train_distributions,
                     pre_computed_distributions);
}

TEST_CASE("Initial_distributions_simple", "[train_dist_calc]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");
  precomputed_distributions_container train_distributions(schedule->node_count);
  start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                        {0.1, 0.8, 0.1}, -1);

  precomputed_distributions_calculator::perform_precomputation(
      *schedule, s_t_distributions, train_distributions);

  for (auto const first_route_node :
       schedule->route_index_to_first_route_node) {
    test_distributions(*first_route_node, train_distributions,
                       distributions_calculator::is_pre_computed_route(
                           *schedule, *first_route_node));
  }
}

#include "motis/reliability/db_distributions.h"
TEST_CASE("Initial_distributions_db_distributions", "[train_dist_calc]") {
  std::cout << "Initial_distributions_db_distributions" << std::endl;
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");
  precomputed_distributions_container train_distributions(schedule->node_count);
  db_distributions db_dists(
      "/home/keyhani/git/motis/DBDists/DBData/20130805/Original/td/", 120,
      120);  // todo: read max travel time from graph

  precomputed_distributions_calculator::perform_precomputation(
      *schedule, db_dists, train_distributions);

  for (auto const first_route_node :
       schedule->route_index_to_first_route_node) {
    test_distributions(*first_route_node, train_distributions,
                       distributions_calculator::is_pre_computed_route(
                           *schedule, *first_route_node));
  }
}

#if 1
TEST_CASE("Initial_distributions_db_distributions2", "[train_dist_calc]") {
  std::cout << "Initial_distributions_db_distributions2" << std::endl;
  auto schedule = load_text_schedule(
      "/data/schedule/2015-08-26_motis_6904ee18-73a0-474e-840b-f49586f4baf1/"
      "schedule/motis");
  std::cout << "schedule loaded" << std::endl;
  precomputed_distributions_container train_distributions(schedule->node_count);
  db_distributions db_dists(
      "/home/keyhani/git/motis/DBDists/DBData/20130805/Original/td/", 120,
      120);  // todo: read max travel time from graph

  precomputed_distributions_calculator::perform_precomputation(
      *schedule, db_dists, train_distributions);

  for (auto const first_route_node :
       schedule->route_index_to_first_route_node) {
    test_distributions(*first_route_node, train_distributions,
                       distributions_calculator::is_pre_computed_route(
                           *schedule, *first_route_node));
  }
}
#endif

TEST_CASE("Test queue element", "[train_dist_calc]") {
  distributions_calculator::queue_type queue;

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
