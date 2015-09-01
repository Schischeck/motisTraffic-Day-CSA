#pragma once

#include <map>
#include <memory>
#include <vector>

#include "motis/reliability/distributions_container.h"

namespace motis {
namespace reliability {

struct precomputed_distributions_test_container
    : precomputed_distributions_container {
  precomputed_distributions_test_container(
      std::vector<probability> const probabilities, int const first_minute)
      : precomputed_distributions_container(0) {
    dist.init(probabilities, first_minute);
  }

  probability_distribution const& get_distribution(
      unsigned int const route_node_idx, unsigned int const light_conn_idx,
      type const t) const override {
    (void)route_node_idx;
    (void)light_conn_idx;
    (void)t;
    return dist;
  }

  bool contains_arrival_distributions(
      unsigned int const route_node_idx) const override {
    return true;
  }

  probability_distribution dist;
};

}  // namespace reliability
}  // namespace motis
