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

struct distributions_container;
struct start_and_travel_distributions;

namespace distributions_calculator {
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

using queue_type =
    std::priority_queue<queue_element, std::vector<queue_element>,
                        queue_element::queue_element_cmp>;

void process_queue(queue_type& queue, distributions_container& container,
                   start_and_travel_distributions const& s_t_distributions,
                   schedule const& schedule);

void compute_dep_and_arr_distribution(
    queue_element const& element, distributions_container& container,
    start_and_travel_distributions const& s_t_distributions,
    schedule const& schedule);

/* check_class: check whether the distributions have to be pre-computed */
template <bool FirstRouteNode, bool CheckClass>
void insert_all_light_connections(node const& tail_node, queue_type& queue,
                                  distributions_container& container,
                                  schedule const& schedule);

void prepare_distributions_container(edge const& route_edge,
                                     distributions_container& container);

bool is_pre_computed_route(schedule const& schedule,
                           node const& first_route_node);

void output_element(std::ostream& os, schedule const& schedule,
                    node const& from, node const& to,
                    light_connection const& light_connection,
                    unsigned short const light_connection_idx,
                    bool const is_first_route_node);
}

namespace precomputed_distributions_calculator {
void perform_precomputation(
    schedule const& schedule,
    start_and_travel_distributions const& s_t_distributions,
    distributions_container& distributions_container);
};

namespace route_distributions_calculator {
void compute_distributions_for_route(
    unsigned int const route_id, schedule const& schedule,
    start_and_travel_distributions const& s_t_distributions,
    distributions_container& distributions_container);
}

}  // namespace reliability
}  // namespace motis
