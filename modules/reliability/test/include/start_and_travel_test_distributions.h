#pragma once

#include <map>
#include <memory>
#include <vector>

#include "motis/reliability/start_and_travel_distributions.h"

namespace motis {
namespace reliability {

struct start_and_travel_test_distributions : start_and_travel_distributions {
  start_and_travel_test_distributions(
      std::vector<probability> const& start_probabilities) {
    init_start_distribution(start_probabilities);
  }

  start_and_travel_test_distributions(
      std::vector<probability> const& traveltime_probabilities,
      int const first_minute, unsigned int const dep_delay_from,
      unsigned int const dep_delay_to) {
    init_travel_distributions(traveltime_probabilities, first_minute,
                              dep_delay_from, dep_delay_to);
  }

  start_and_travel_test_distributions(
      std::vector<probability> const& start_probabilities,
      std::vector<probability> const& traveltime_probabilities,
      int const first_minute, unsigned int const dep_delay_from,
      unsigned int const dep_delay_to) {
    init_start_distribution(start_probabilities);
    init_travel_distributions(traveltime_probabilities, first_minute,
                              dep_delay_from, dep_delay_to);
  }

  probability_distribution const& get_start_distribution(
      std::string const& family) const override {
    (void)family;
    return start_distribution_;
  }

  void get_travel_time_distributions(
      std::string const& family, unsigned int const travel_time,
      std::vector<travel_time_distribution>& distributions) const override {
    (void)family;
    (void)travel_time;
    distributions.push_back(*travel_time_distribution_);
  }

  probability_distribution start_distribution_;
  std::unique_ptr<travel_time_distribution> travel_time_distribution_;

private:
  void init_start_distribution(
      std::vector<probability> const& start_probabilities) {
    start_distribution_.init(start_probabilities, 0);
  }

  void init_travel_distributions(
      std::vector<probability> const& traveltime_probabilities,
      int const first_minute, unsigned int const dep_delay_from,
      unsigned int const dep_delay_to) {
    travel_distribution_.init(traveltime_probabilities, first_minute);
    travel_time_distribution_ =
        std::unique_ptr<travel_time_distribution>(new travel_time_distribution(
            travel_distribution_, dep_delay_from, dep_delay_to));
  }

  probability_distribution travel_distribution_;
};

}  // namespace reliability
}  // namespace motis
