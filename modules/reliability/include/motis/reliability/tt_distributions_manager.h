#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "motis/core/schedule/time.h"

#include "motis/reliability/probability_distribution.h"

namespace motis {
namespace reliability {

struct tt_distributions_manager {

  struct generated_distribution_info {
    /* travel-time distributions indexed by departure delay */
    std::vector<probability_distribution> distributions;
    /** minimum and maximum allowed travel delay of the
     * travel time distribution depending on the scheduled travel time */
    int min_travel_delay;
    int max_travel_delay;
    unsigned int max_departure_delay_;
  } generated_distributions_info_;

  tt_distributions_manager();

  probability_distribution const& get_start_distribution(
      std::string const& train_category) const {
    return start_distributions_;
  }

  std::tuple<std::vector<probability_distribution> const&, unsigned int, int,
             int>
  get_travel_time_distributions(std::string const& train_category,
                                duration const travel_time) const;

  duration const longest_travel_time_ = 300;

private:
  void init_generated_distributions();

  static void generate_distribution(probability_distribution& distribution,
                                    unsigned int const depature_delay,
                                    unsigned int const travel_duration);

  probability_distribution start_distributions_;

  std::vector<generated_distribution_info> generated_distributions;
};

}  // namespace reliability
}  // namespace motis
