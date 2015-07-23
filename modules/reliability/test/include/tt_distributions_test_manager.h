#pragma once

#include <map>
#include <vector>

#include "motis/reliability/tt_distributions_manager.h"

namespace motis {
namespace reliability {

struct tt_distributions_test_manager : tt_distributions_manager {

private:
  void init_start_distributions() {
    probability_distribution distribution;
    distribution.init(std::vector<probability>() = {0.6, 0.4}, 0);
    distribution_class_to_start_distribution_["FV"] = distribution;
    distribution_class_to_start_distribution_["RV"] = distribution;
  }

  void generate_distribution(
      probability_distribution& distribution, unsigned int const depature_delay,
      unsigned int const travel_duration) {
    (void)depature_delay;
    (void)travel_duration;
    distribution.init(std::vector<probability>() = {0.1, 0.8, 0.1}, -1);
  }
};

}  // namespace reliability
}  // namespace motis
