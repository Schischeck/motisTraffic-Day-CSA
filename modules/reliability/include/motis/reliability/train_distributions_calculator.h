#pragma once

#include <queue>
#include <tuple>

#include "motis/core/schedule/Connection.h"

namespace td {
struct Schedule;
class Node;
}

namespace motis {
namespace reliability {

struct queue_element {
  queue_element(td::LightConnection const* light_connection, td::Node* from,
                td::Node* to)
      : light_connection_(light_connection), from_(from), to_(to) {}

  td::LightConnection const* light_connection_;
  td::Node* from_;
  td::Node* to_;
};

class queue_element_cmp {
public:
  bool operator()(queue_element const& a, queue_element const& b) {
    return a.light_connection_->dTime < b.light_connection_->dTime;
  }
};

class train_distributions_calculator {
public:
  train_distributions_calculator(td::Schedule* schedule);

  bool calculate_initial_distributions();

private:
  bool insert_first_route_elements_into_queue();

  bool process_queue();

  td::Schedule* schedule_;

  std::priority_queue<queue_element, std::vector<queue_element>,
                      queue_element_cmp> queue_;
};

}  // namespace reliability
}  // namespace motis
