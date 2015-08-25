#pragma once

#include "motis/reliability/db_distributions_loader.h"
#include "start_and_travel_distributions.h"

namespace motis {
namespace reliability {

struct probability_distribution;

struct db_distributions : start_and_travel_distributions {
  struct distribution_mapping {
    distribution_mapping(probability_distribution const& distribution,
                         unsigned int const travel_time,
                         unsigned int const delay)
        : distribution_(distribution),
          travel_time_(travel_time),
          delay_(delay) {}
    probability_distribution const& distribution_;
    unsigned int const travel_time_;
    unsigned int const delay_;
  };

  db_distributions(std::string const root,
                   unsigned int const max_expected_travel_time,
                   unsigned int const max_expected_departure_delay);

  probability_distribution const& get_start_distribution(
      std::string const& family) const override;

  /**
   * Returns vector containing pairs of departure-delay and
   * probability-distribution.
   */
  void get_travel_time_distributions(
      std::string const& family, unsigned int const travel_time,
      unsigned int const to_departure_delay,
      std::vector<probability_distribution_cref>& distributions) const override;

private:
  std::string const& get_distribution_class(std::string const& family) const;

  std::map<std::string, std::string> family_to_distribution_class_;

  std::vector<std::pair<unsigned int, probability_distribution>>
      all_probability_distributions_;

  // TODO: enable access in constant time (a vector for each travel time)
  std::map<std::string, /* distribution class */
           std::vector<distribution_mapping>> distribution_mappings_;

  std::map<std::string, probability_distribution>
      class_to_probability_distributions_;

  probability_distribution default_start_distribution_;
  std::string const empty_string_;
};  // struct db_distributions

namespace db_distributions_helpers {
void get_distributions(
    unsigned int const travel_time, unsigned int const to_departure_delay,
    std::vector<db_distributions::distribution_mapping> const& all_mappings,
    std::vector<start_and_travel_distributions::probability_distribution_cref>&
        distributions);
}

}  // namespace reliability
}  // namespace motis
