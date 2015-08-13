#define CATCH_CONFIG_MAIN

#include "catch/catch.hpp"

#include "motis/loader/loader.h"

#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/schedule.h"

#include "motis/reliability/train_distributions_calculator.h"
#include "motis/reliability/train_distributions.h"

#include "include/tt_distributions_test_manager.h"

using namespace motis;
using namespace motis::reliability;

TEST_CASE("Initial distributions", "[train_dist_calc]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");
  train_distributions_container train_distributions(schedule->node_count);
  tt_distributions_test_manager tt_distributions({0.8, 0.2}, {0.1, 0.8, 0.1},
                                                 -1, 50);

  train_distributions_calculator calculator(*schedule, train_distributions,
                                            tt_distributions);
  calculator.calculate_initial_distributions();
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
