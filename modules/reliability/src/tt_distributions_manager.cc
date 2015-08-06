#include "motis/reliability/tt_distributions_manager.h"

#include <iostream>
#include <cassert>

#include "motis/reliability/probability_distribution.h"

namespace motis {
namespace reliability {

tt_distributions_manager::tt_distributions_manager() {
  default_start_distribution_.init_one_point(0, 1.0);
}

void tt_distributions_manager::initialize() {
  init_distribution_classes();
  init_start_distributions();
  init_generated_distributions();
}

void tt_distributions_manager::init_start_distributions() {}

probability_distribution const&
tt_distributions_manager::get_start_distribution(
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

travel_distribution_info const&
tt_distributions_manager::get_travel_time_distributions(
    std::string const& train_category, duration const travel_time) const {
  assert(travel_time < longest_travel_time_);
  return travel_time_to_generated_distribution_[travel_time];  // XXX category
}

void tt_distributions_manager::init_distribution_classes() {
  train_category_to_distribution_class_["ICE"] = "FV";
  train_category_to_distribution_class_["IC"] = "FV";
  train_category_to_distribution_class_["RE"] = "RV";
  train_category_to_distribution_class_["RB"] = "RV";
  train_category_to_distribution_class_["S"] = "S";
}

void tt_distributions_manager::init_generated_distributions() {
  travel_time_to_generated_distribution_.resize(longest_travel_time_);

  // for each train-edge length there exist another distribution
  for (unsigned int t = 0; t < longest_travel_time_; t++) {
    std::vector<probability_distribution> distributions;
    // TODO: read from db-distributions
    unsigned int max_departure_delay = 300;
    int min_travel_delay = 0;
    int max_travel_delay = 0;

    for (unsigned int d = 0; d <= max_departure_delay; d++) {
      probability_distribution distribution;
      generate_distribution(distribution, d, t);
      distributions.push_back(distribution);

      if (distribution.first_minute() < min_travel_delay)
        min_travel_delay = distribution.first_minute();
      if (distribution.last_minute() > max_travel_delay)
        max_travel_delay = distribution.last_minute();
    }
    travel_time_to_generated_distribution_.push_back(
        travel_distribution_info(distributions, max_departure_delay,
                                 min_travel_delay, max_travel_delay));
  }
}

void tt_distributions_manager::generate_distribution(
    probability_distribution& distribution, unsigned int const depature_delay,
    unsigned int const travel_duration) {
  distribution.init_one_point(0, 1.0);  // TODO
}

}  // namespace reliability
}  // namespace motis
