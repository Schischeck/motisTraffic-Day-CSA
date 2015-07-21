#pragma once

#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "motis/core/schedule/time.h"

#include "motis/reliability/probability_distribution.h"

namespace motis {
namespace reliability {

struct tt_distributions_manager {

  tt_distributions_manager();
  virtual ~tt_distributions_manager() {}
  void initialize();

  probability_distribution const& get_start_distribution(
      std::string const& train_category) const {
    auto const class_it =
        train_category_to_distribution_class_.find(train_category);
    if (class_it != train_category_to_distribution_class_.end()) {
      auto const& distribution_class = class_it->second;
      auto const dist_it =
          distribution_class_to_start_distribution_.find(distribution_class);
      if (dist_it != distribution_class_to_start_distribution_.end()) {
        return dist_it->second;
      }
    }
    return default_start_distribution_;
  }

  std::tuple<std::vector<probability_distribution> const&, unsigned int, int,
             int>
  get_travel_time_distributions(std::string const& train_category,
                                duration const travel_time) const;

  duration const longest_travel_time_ = 300;

protected:
  void init_distribution_classes();
  virtual void init_start_distributions();
  void init_generated_distributions();

  virtual void generate_distribution(probability_distribution& distribution,
                                     unsigned int const depature_delay,
                                     unsigned int const travel_duration);

  std::map<std::string, std::string> train_category_to_distribution_class_;

  /** Mapping from train class (RV, FV, ...) to the start distributions */
  std::map<std::string, probability_distribution>
      distribution_class_to_start_distribution_;
  probability_distribution default_start_distribution_;

  struct generated_distribution_info {
    /* travel-time distributions indexed by departure delay */
    std::vector<probability_distribution> distributions;
    /** minimum and maximum allowed travel delay of the
     * travel time distribution depending on the scheduled travel time */
    int min_travel_delay;
    int max_travel_delay;
    unsigned int max_departure_delay_;
  };

  std::vector<generated_distribution_info>
      travel_time_to_generated_distribution_;
};

}  // namespace reliability
}  // namespace motis
