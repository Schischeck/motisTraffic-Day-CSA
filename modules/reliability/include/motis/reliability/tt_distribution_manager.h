#pragma once

namespace motis {
namespace reliability {

#include "motis/reliability/probability_distribution.h"

struct tt_distribution_manager {

  probability_distribution const& get_start_distribution(
      std::string const& train_category) const {
    return start_dist;
  }

private:
  probability_distribution start_dist;
};

}  // namespace reliability
}  // namespace motis
