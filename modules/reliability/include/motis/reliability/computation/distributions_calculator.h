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
      if (a.light_connection_->d_time_ == b.light_connection_->d_time_) {
        return a.from_->id_ > b.from_->id_;
      }
      return a.light_connection_->d_time_ > b.light_connection_->d_time_;
    }
  };
  queue_element(node const* from, node const* to,
                light_connection const* light_connection,
                uint16_t const light_connection_idx,
                bool const is_first_route_node)
      : from_(from),
        to_(to),
        light_connection_(light_connection),
        light_connection_idx_(light_connection_idx),
        is_first_route_node_(is_first_route_node) {}
  node const* from_;
  node const* to_;
  light_connection const* light_connection_;
  uint16_t light_connection_idx_;
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
std::tuple<bool, time, time> get_feeder_time_interval(
    time const departure_time, duration change_time,
    duration const feeder_threshold);
std::vector<std::pair<node const*, light_connection const*>> get_feeders(
    node const& route_node, light_connection const& light_conn,
    schedule const& sched);
}  // namespace detail
}  // namespace precomputation
}  // namespace distributions_calculator
}  // namespace reliability
}  // namespace motis
