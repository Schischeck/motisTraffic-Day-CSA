#pragma once

#include "motis/reliability/db_distributions_loader.h"
#include "start_and_travel_distributions.h"

namespace motis {
namespace reliability {

struct probability_distribution;

struct db_distributions : start_and_travel_distributions {
  db_distributions(std::string const root);

  probability_distribution const& get_start_distribution(
      std::string const& family) const override;

  /**
   * Returns vector containing pairs of departure-delay and
   * probability-distribution.
   */
  void get_travel_time_distributions(
      std::string const& family, unsigned int const travel_time,
      std::vector<travel_time_distribution>& distributions) const override;

private:
  std::string const& get_distribution_class(std::string const& family) const;

  std::map<std::string, std::string> family_to_distribution_class_;

  std::vector<std::pair<unsigned int, probability_distribution> >
      all_probability_distributions_;

  std::map<std::string, /* distribution class */
           std::vector<db_distributions_loader::distribution_mapping<
               probability_distribution const&> > > distribution_mappings_;

  std::map<std::string, probability_distribution>
      class_to_probability_distributions_;

  probability_distribution default_start_distribution_;
  std::string const empty_string_;
};  // struct db_distributions

}  // namespace reliability
}  // namespace motis
