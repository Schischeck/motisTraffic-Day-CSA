#pragma once

#include <map>
#include <memory>
#include <vector>

#include "motis/core/schedule/nodes.h"

#include "motis/reliability/computation/distributions_calculator.h"
#include "motis/reliability/distributions/distributions_container.h"

namespace motis {
namespace reliability {
namespace distributions_container {

struct test_container : container {
  test_container(std::vector<probability> const probabilities,
                 int const first_minute) {
    dist.init(probabilities, first_minute);
  }

  probability_distribution const& get_distribution(key const&) const override {
    return dist;
  }

  bool contains_distribution(key const&) const override { return true; }

  probability_distribution dist;
};

inline distributions_container::container::node const&
init_feeders_and_get_distribution_node(
    distributions_container::container& feeder_distributions,
    node const& route_node, light_connection const& light_conn,
    std::vector<probability> const& values, int const first_minute,
    schedule const& sched) {
  auto& train_distribution_node = feeder_distributions.get_node_non_const(
      distributions_container::to_container_key(route_node, light_conn,
                                                time_util::departure, sched));
  for (auto const feeder :
       distributions_calculator::precomputation::detail::get_feeders(
           route_node, light_conn, sched)) {
    auto& feeder_node = feeder_distributions.get_node_non_const(
        distributions_container::to_container_key(*feeder.first, *feeder.second,
                                                  time_util::arrival, sched));
    feeder_node.pd_.init(values, first_minute);
    feeder_node.successors_.push_back(&train_distribution_node);
    train_distribution_node.predecessors_.push_back(&feeder_node);
  }

  return train_distribution_node;
}
}
}  // namespace reliability
}  // namespace motis
