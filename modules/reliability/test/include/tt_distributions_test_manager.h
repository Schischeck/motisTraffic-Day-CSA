#pragma once

#include <map>
#include <memory>
#include <vector>

#include "motis/reliability/tt_distributions_manager.h"

namespace motis {
namespace reliability {

struct tt_distributions_test_manager : tt_distributions_manager {
  tt_distributions_test_manager(
      std::vector<probability> const& start_probabilities) {
    init_start_distribution(start_probabilities);
  }

  tt_distributions_test_manager(
      std::vector<probability> const& traveltime_probabilities,
      int const first_minute, unsigned int const max_departure_delay) {
    init_travel_distributions(traveltime_probabilities, first_minute,
                              max_departure_delay);
  }

  tt_distributions_test_manager(
      std::vector<probability> const& start_probabilities,
      std::vector<probability> const& traveltime_probabilities,
      int const first_minute, unsigned int const max_departure_delay) {
    init_start_distribution(start_probabilities);
    init_travel_distributions(traveltime_probabilities, first_minute,
                              max_departure_delay);
  }

  probability_distribution const& get_start_distribution(
      std::string const& train_category) const override {
    (void)train_category;
    return start_distribution_;
  }

  travel_distribution_info const& get_travel_time_distributions(
      std::string const& train_category,
      duration const travel_time) const override {
    (void)train_category;
    (void)travel_time;
    return *travel_distribution_;
  }

  probability_distribution start_distribution_;
  std::unique_ptr<travel_distribution_info> travel_distribution_;

private:
  void init_start_distribution(
      std::vector<probability> const& start_probabilities) {
    start_distribution_.init(start_probabilities, 0);
  }

  void init_travel_distributions(
      std::vector<probability> const& traveltime_probabilities,
      int const first_minute, unsigned int const max_departure_delay) {
    std::vector<probability_distribution> distributions;
    for (unsigned int i = 0; i <= max_departure_delay; i++) {
      probability_distribution pd;
      pd.init(traveltime_probabilities, first_minute);
      distributions.push_back(pd);
    }
    travel_distribution_ =
        std::unique_ptr<travel_distribution_info>(new travel_distribution_info(
            distributions, max_departure_delay, first_minute,
            (first_minute + traveltime_probabilities.size()) - 1));
  }
};

}  // namespace reliability
}  // namespace motis
