#pragma once

#include <queue>
#include <tuple>

#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/nodes.h"

#include "motis/reliability/distributions/distributions_container.h"

namespace motis {
struct schedule;
class node;

namespace reliability {
struct context;
struct probability_distribution;
struct start_and_travel_distributions;

namespace distributions_calculator {
namespace common {
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

void compute_dep_and_arr_distribution(
    queue_element const&,
    distributions_container::container::node const& departing_distribution_node,
    probability_distribution& departure_distribution,
    probability_distribution& arrival_distribution, context const&,
    distributions_container::container const& train_distributions_container);
}  // namespace common

namespace precomputation {
void perform_precomputation(schedule const&,
                            start_and_travel_distributions const&,
                            distributions_container::container&);

namespace detail {
bool is_pre_computed_route(schedule const& schedule,
                           node const& first_route_node);
}  // namespace detail
}  // namespace precomputation
}  // namespace distributions_calculator
}  // namespace reliability
}  // namespace motis
