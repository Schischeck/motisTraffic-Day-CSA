#define CATCH_CONFIG_MAIN

#include "catch/catch.hpp"

#include "motis/reliability/train_distributions_calculator.h"
#include "motis/core/schedule/nodes.h"

using namespace motis;
using namespace motis::reliability;

TEST_CASE("Test queue", "[train_dist_calc]") {

  std::priority_queue<queue_element, std::vector<queue_element>,
                      queue_element_cmp> queue;

  node dummy_node(nullptr, 0);

  light_connection lc1(1);
  light_connection lc2(2);
  light_connection lc3(3);

  queue.emplace(&dummy_node, &dummy_node, &lc3, 0, false);
  queue.emplace(&dummy_node, &dummy_node, &lc2, 0, false);
  queue.emplace(&dummy_node, &dummy_node, &lc1, 0, false);
  queue.emplace(&dummy_node, &dummy_node, &lc3, 0, false);

  REQUIRE(queue.top().light_connection_->d_time == 3);
  queue.pop();
  REQUIRE(queue.top().light_connection_->d_time == 3);
  queue.pop();
  REQUIRE(queue.top().light_connection_->d_time == 2);
  queue.pop();

  queue.emplace(&dummy_node, &dummy_node, &lc2, 0, false);

  REQUIRE(queue.top().light_connection_->d_time == 2);
  queue.pop();
  REQUIRE(queue.top().light_connection_->d_time == 1);
}

TEST_CASE("Test first departures", "[train_dist_calc]") {}
