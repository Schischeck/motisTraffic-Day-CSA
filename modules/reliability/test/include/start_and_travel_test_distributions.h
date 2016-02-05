#pragma once

#include <map>
#include <memory>
#include <vector>

#include "motis/reliability/distributions/start_and_travel_distributions.h"

namespace motis {
namespace reliability {

struct start_and_travel_test_distributions : start_and_travel_distributions {
  start_and_travel_test_distributions(
      std::vector<probability> const& start_probabilities) {
    init_start_distribution(start_probabilities);
    init_travel_distributions({1.0}, 0);
  }

  start_and_travel_test_distributions(
      std::vector<probability> const& traveltime_probabilities,
      int const first_minute) {
    init_start_distribution({1.0});
    init_travel_distributions(traveltime_probabilities, first_minute);
  }

  start_and_travel_test_distributions(
      std::vector<probability> const& start_probabilities,
      std::vector<probability> const& traveltime_probabilities,
      int const first_minute) {
    init_start_distribution(start_probabilities);
    init_travel_distributions(traveltime_probabilities, first_minute);
  }

  std::pair<bool, probability_distribution_cref> get_start_distribution(
      std::string const& family) const override {
    (void)family;
    return std::make_pair(true, std::cref(start_distribution_));
  }

  bool get_travel_time_distributions(std::string const& family,
                                     unsigned int const travel_time,
                                     unsigned int const to_departure_delay,
                                     std::vector<probability_distribution_cref>&
                                         distributions) const override {
    (void)family;
    (void)travel_time;
    for (unsigned int d = 0; d <= to_departure_delay; d++) {
      distributions.push_back(std::cref(travel_distribution_));
    }
    return true;
  }

  probability_distribution start_distribution_;
  probability_distribution travel_distribution_;

private:
  void init_start_distribution(
      std::vector<probability> const& start_probabilities) {
    start_distribution_.init(start_probabilities, 0);
    travel_distribution_.init_one_point(0, 1.0);
  }

  void init_travel_distributions(
      std::vector<probability> const& traveltime_probabilities,
      int const first_minute) {
    travel_distribution_.init(traveltime_probabilities, first_minute);
  }
};

}  // namespace reliability
}  // namespace motis
