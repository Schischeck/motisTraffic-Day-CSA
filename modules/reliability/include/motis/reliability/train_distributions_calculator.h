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

struct train_distributions_container;

struct queue_element {
  queue_element(td::Node* from, td::Node* to,
                td::LightConnection const* light_connection,
                unsigned short light_connection_idx,
                bool const is_first_route_node)
      : from_(from),
        to_(to),
        light_connection_(light_connection),
        light_connection_idx_(light_connection_idx),
        is_first_route_node_(is_first_route_node) {}

  td::Node* from_;
  td::Node* to_;
  td::LightConnection const* light_connection_;
  unsigned short light_connection_idx_;
  bool is_first_route_node_;
};

class queue_element_cmp {
public:
  bool operator()(queue_element const& a, queue_element const& b) {
    return a.light_connection_->dTime < b.light_connection_->dTime;
  }
};

class train_distributions_calculator {
public:
  train_distributions_calculator(
      td::Schedule* schedule,
      train_distributions_container& distributions_container);

  bool calculate_initial_distributions();

private:
  bool insert_first_route_elements_into_queue();

  bool process_element(queue_element element);

  td::Schedule* schedule_;

  train_distributions_container& distributions_container_;

  std::priority_queue<queue_element, std::vector<queue_element>,
                      queue_element_cmp> queue_;
};

}  // namespace reliability
}  // namespace motis
