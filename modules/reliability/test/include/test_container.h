#pragma once

#include <map>
#include <memory>
#include <vector>

#include "motis/reliability/distributions/distributions_container.h"

namespace motis {
namespace reliability {
namespace distributions_container {

struct test_container : container {
  test_container(
      std::vector<probability> const probabilities, int const first_minute) {
    dist.init(probabilities, first_minute);
  }

  probability_distribution const& get_distribution(key const&) const override {
    return dist;
  }

  bool contains_distribution(key const&) const override { return true; }

  probability_distribution dist;
};
}
}  // namespace reliability
}  // namespace motis
