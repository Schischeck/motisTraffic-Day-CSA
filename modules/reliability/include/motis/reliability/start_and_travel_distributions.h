#pragma once

#include "start_and_travel_distributions.h"

#include <string>
#include <vector>

#include "motis/reliability/db_distributions_loader.h"

namespace motis {
namespace reliability {

struct probability_distribution;

struct start_and_travel_distributions {
  struct travel_time_distribution {
    travel_time_distribution(probability_distribution const& distribution,
                             unsigned int const departure_delay_from,
                             unsigned int const departure_delay_to)
        : distribution_(distribution),
          departure_delay_from_(departure_delay_from),
          departure_delay_to_(departure_delay_to) {}
    probability_distribution const& distribution_;
    unsigned int const departure_delay_from_;
    unsigned int const departure_delay_to_;
  };

  virtual ~start_and_travel_distributions() {}

  virtual probability_distribution const& get_start_distribution(
      std::string const& family) const = 0;

  /**
   * Returns vector containing pairs of departure-delay and
   * probability-distribution.
   */
  virtual void get_travel_time_distributions(
      std::string const& family, unsigned int const travel_time,
      std::vector<travel_time_distribution>& distributions) const = 0;
};

}  // namespace reliability
}  // namespace motis
