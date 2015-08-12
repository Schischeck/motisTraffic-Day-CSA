#pragma once

#include <queue>
#include <tuple>

#include "motis/core/schedule/connection.h"

namespace motis {

struct schedule;
class node;

namespace reliability {

struct train_distributions_container;
struct tt_distributions_manager;

struct queue_element {
  queue_element(node* from, node* to, light_connection const* light_connection,
                unsigned short light_connection_idx,
                bool const is_first_route_node)
      : from_(from),
        to_(to),
        light_connection_(light_connection),
        light_connection_idx_(light_connection_idx),
        is_first_route_node_(is_first_route_node) {}

  node* from_;
  node* to_;
  light_connection const* light_connection_;
  unsigned short light_connection_idx_;
  bool is_first_route_node_;
};

class queue_element_cmp {
public:
  bool operator()(queue_element const& a, queue_element const& b) {
    return a.light_connection_->d_time < b.light_connection_->d_time;
  }
};

class train_distributions_calculator {
public:
  train_distributions_calculator(
      schedule& schedule,
      train_distributions_container& distributions_container,
      tt_distributions_manager& tt_dist_manager);

  void calculate_initial_distributions();

private:
  void process_element(queue_element element);

  void insert_all_light_connections(node& tail_node,
                                    bool const is_first_route_node);

  schedule& schedule_;

  train_distributions_container& distributions_container_;

  tt_distributions_manager& tt_distributions_manager_;

  std::priority_queue<queue_element, std::vector<queue_element>,
                      queue_element_cmp> queue_;
};

}  // namespace reliability
}  // namespace motis
