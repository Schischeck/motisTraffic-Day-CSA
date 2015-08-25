
#define CATCH_CONFIG_MAIN

#include "catch/catch.hpp"

#include "motis/loader/loader.h"

#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/schedule.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/train_distributions_calculator.h"
#include "motis/reliability/train_distributions.h"

#include "include/start_and_travel_test_distributions.h"

using namespace motis;
using namespace motis::reliability;

void test_distributions(node const& route_node,
                        train_distributions_container& train_distributions,
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
        train_distributions.get_train_distributions(
            route_node._id, train_distributions_container::departure);
    auto const& arrival_distributions =
        train_distributions.get_train_distributions(
            head_route_node._id, train_distributions_container::arrival);

    for (unsigned int l = 0; l < route_edge->_m._route_edge._conns.size();
         l++) {
      REQUIRE_FALSE(departure_distributions.get_distribution(l).empty());
      REQUIRE(departure_distributions.get_distribution(l).sum());
      REQUIRE_FALSE(arrival_distributions.get_distribution(l).empty());
      REQUIRE(arrival_distributions.get_distribution(l).sum());
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
  std::cout << "Initial_distributions_simple" << std::endl;
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");
  train_distributions_container train_distributions(schedule->node_count);
  start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                        {0.1, 0.8, 0.1}, -1);

  std::cout << "calculator" << std::endl;
  train_distributions_calculator calculator(*schedule, train_distributions,
                                            s_t_distributions);
  calculator.calculate_initial_distributions();

  std::cout << "test" << std::endl;

  for (auto const first_route_node :
       schedule->route_index_to_first_route_node) {
    test_distributions(*first_route_node, train_distributions,
                       train_distributions_calculator::compute_distributions(
                           *schedule, *graph_accessor::get_departing_route_edge(
                                          *first_route_node)));
  }

  std::cout << "finished" << std::endl;
}

#include "motis/reliability/db_distributions.h"
TEST_CASE("Initial_distributions_db_distributions", "[train_dist_calc]") {
  std::cout << "Initial_distributions_db_distributions" << std::endl;
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");
  train_distributions_container train_distributions(schedule->node_count);
  db_distributions db_dists(
      "/home/keyhani/git/motis/DBDists/DBData/20130805/Original/td/");

  train_distributions_calculator calculator(*schedule, train_distributions,
                                            db_dists);
  calculator.calculate_initial_distributions();

  for (auto const first_route_node :
       schedule->route_index_to_first_route_node) {
    test_distributions(*first_route_node, train_distributions,
                       train_distributions_calculator::compute_distributions(
                           *schedule, *graph_accessor::get_departing_route_edge(
                                          *first_route_node)));
  }
}

TEST_CASE("Test queue element", "[train_dist_calc]") {
  std::priority_queue<
      train_distributions_calculator::queue_element,
      std::vector<train_distributions_calculator::queue_element>,
      train_distributions_calculator::queue_element::queue_element_cmp> queue;

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
