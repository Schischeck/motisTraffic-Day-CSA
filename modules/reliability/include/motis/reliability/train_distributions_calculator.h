#pragma once

#include <queue>
#include <tuple>

#include "motis/core/schedule/connection.h"

namespace motis {

struct schedule;
class node;
class edge;

namespace reliability {

struct train_distributions_container;
struct tt_distributions_manager;

struct train_distributions_calculator {

  struct queue_element {
    struct queue_element_cmp {
      bool operator()(queue_element const& a, queue_element const& b) {
        return a.light_connection_->d_time < b.light_connection_->d_time;
      }
    };

    queue_element(node const* from, node const* to,
                  light_connection const* light_connection,
                  unsigned short const light_connection_idx,
                  bool const is_first_route_node)
        : from_(from),
          to_(to),
          light_connection_(light_connection),
          light_connection_idx_(light_connection_idx),
          is_first_route_node_(is_first_route_node) {}

    node const* from_;
    node const* to_;
    light_connection const* light_connection_;
    unsigned short light_connection_idx_;
    bool is_first_route_node_;
  };

  train_distributions_calculator(
      schedule const& schedule,
      train_distributions_container& distributions_container,
      tt_distributions_manager const& tt_dist_manager);

  void calculate_initial_distributions();

private:
  void process_element(queue_element element);
  void insert_all_light_connections(node const& tail_node,
                                    bool const is_first_route_node);
  void manage_train_distributions(edge const& route_edge);

  schedule const& schedule_;
  tt_distributions_manager const& tt_distributions_manager_;
  train_distributions_container& distributions_container_;

  std::priority_queue<queue_element, std::vector<queue_element>,
                      queue_element::queue_element_cmp> queue_;
};

}  // namespace reliability
}  // namespace motis
