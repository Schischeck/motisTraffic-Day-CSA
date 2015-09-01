#pragma once

#include <queue>
#include <tuple>

#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/nodes.h"

namespace motis {

struct schedule;
class node;
class edge;

namespace reliability {

struct train_distributions_container;
struct start_and_travel_distributions;

struct train_distributions_calculator {

  struct queue_element {
    struct queue_element_cmp {
      bool operator()(queue_element const& a, queue_element const& b) {
        if (a.light_connection_->d_time == b.light_connection_->d_time) {
          return a.from_->_id > b.from_->_id;
        }
        return a.light_connection_->d_time > b.light_connection_->d_time;
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
      start_and_travel_distributions const& s_t_dist_manager);

  void calculate_initial_distributions();

  static bool is_pre_computed_route(schedule const& schedule,
                                    node const& first_route_node);

  static void output_element(std::ostream& os, schedule const& schedule,
                             node const& from, node const& to,
                             light_connection const& light_connection,
                             unsigned short const light_connection_idx,
                             bool const is_first_route_node);

private:
  void insert_into_queue(node const* from, node const* to,
                         light_connection const* light_connection,
                         unsigned short const light_connection_idx,
                         bool const is_first_route_node);
  void process_element(queue_element const& element);
  /* check_class: check whether the distributions have to be pre-computed */
  template <bool FirstRouteNode, bool CheckClass>
  void insert_all_light_connections(node const& tail_node);
  void prepare_distributions_container(edge const& route_edge);

  schedule const& schedule_;
  start_and_travel_distributions const& s_t_distributions_;
  train_distributions_container& distributions_container_;

  std::priority_queue<queue_element, std::vector<queue_element>,
                      queue_element::queue_element_cmp> queue_;
};

}  // namespace reliability
}  // namespace motis
