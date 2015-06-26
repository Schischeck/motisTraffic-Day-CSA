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

  queue.emplace(lc3, dummyNode, dummyNode);
  queue.emplace(lc2, dummyNode, dummyNode);
  queue.emplace(lc1, dummyNode, dummyNode);
  queue.emplace(lc3, dummyNode, dummyNode);

  REQUIRE(queue.top().light_connection_->dTime == 3);
  queue.pop();
  REQUIRE(queue.top().light_connection_->dTime == 3);
  queue.pop();
  REQUIRE(queue.top().light_connection_->dTime == 2);
  queue.pop();

  queue.emplace(lc2, dummyNode, dummyNode);

  REQUIRE(queue.top().light_connection_->dTime == 2);
  queue.pop();
  REQUIRE(queue.top().light_connection_->dTime == 1);
  queue.pop();
}
