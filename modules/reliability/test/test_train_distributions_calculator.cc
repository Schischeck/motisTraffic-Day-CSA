#include "catch/catch.hpp"

#include "motis/reliability/train_distributions_calculator.h"
#include "motis/core/schedule/Nodes.h"

using namespace motis::reliability;

TEST_CASE("Test queue", "[train_dist_calc]") {

  std::priority_queue<queue_element, std::vector<queue_element>,
                      queue_element_cmp> queue;

  td::Node dummyNode(nullptr, 0);

  td::LightConnection lc1(1);
  td::LightConnection lc2(2);
  td::LightConnection lc3(3);

  queue.emplace(&dummyNode, &dummyNode, &lc3, 0, false);
  queue.emplace(&dummyNode, &dummyNode, &lc2, 0, false);
  queue.emplace(&dummyNode, &dummyNode, &lc1, 0, false);
  queue.emplace(&dummyNode, &dummyNode, &lc3, 0, false);

  REQUIRE(queue.top().light_connection_->dTime == 3);
  queue.pop();
  REQUIRE(queue.top().light_connection_->dTime == 3);
  queue.pop();
  REQUIRE(queue.top().light_connection_->dTime == 2);
  queue.pop();

  queue.emplace(&dummyNode, &dummyNode, &lc2, 0, false);

  REQUIRE(queue.top().light_connection_->dTime == 2);
  queue.pop();
  REQUIRE(queue.top().light_connection_->dTime == 1);
}

TEST_CASE("Test first departures", "[train_dist_calc]") {

}
