#pragma once

#include "start_and_travel_distributions.h"

#include <functional>
#include <string>
#include <vector>

#include "motis/reliability/distributions/db_distributions_loader.h"

namespace motis {
namespace reliability {

struct probability_distribution;

struct start_and_travel_distributions {
  using probability_distribution_cref =
      std::reference_wrapper<probability_distribution const>;

  virtual ~start_and_travel_distributions() {}

  virtual probability_distribution const& get_start_distribution(
      std::string const& family) const = 0;

  /* For each departure delay from 0 to 'to_departure_delay',
   * this method returns a travel-time distributions for a travel with a train
   * of class as stored in 'family' with a travel-time as stored in
   * 'travel_time' */
  virtual void get_travel_time_distributions(
      std::string const& family, unsigned int const travel_time,
      unsigned int const to_departure_delay,
      std::vector<probability_distribution_cref>& distributions) const = 0;
};

}  // namespace reliability
}  // namespace motis
