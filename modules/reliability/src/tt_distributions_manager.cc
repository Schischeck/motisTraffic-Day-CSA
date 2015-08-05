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

void tt_distributions_manager::init_distribution_classes() {
  train_category_to_distribution_class_["ICE"] = "FV";
  train_category_to_distribution_class_["IC"] = "FV";
  train_category_to_distribution_class_["RE"] = "RV";
  train_category_to_distribution_class_["RB"] = "RV";
  train_category_to_distribution_class_["S"] = "S";
}

void tt_distributions_manager::init_start_distributions() {}

std::tuple<std::vector<probability_distribution> const&, unsigned int, int, int>
tt_distributions_manager::get_travel_time_distributions(
    std::string const& train_category, duration const travel_time) const {
  assert(travel_time < longest_travel_time_);
  auto const& distribution =
      travel_time_to_generated_distribution_[travel_time]; // XXX category
  std::cout << "dist: " << distribution.distributions.size() << std::endl;
  return std::make_tuple(
      distribution.distributions, distribution.max_departure_delay_,
      distribution.min_travel_delay, distribution.max_travel_delay);
}

void tt_distributions_manager::init_generated_distributions() {
  travel_time_to_generated_distribution_.resize(longest_travel_time_);

  // for each train-edge length there exist another distribution
  for (unsigned int t = 0; t < longest_travel_time_; t++) {
    auto& info = travel_time_to_generated_distribution_[t];
    info.max_departure_delay_ = 300;  // TODO: read from db-distributions
    info.distributions.resize(info.max_departure_delay_);
    info.min_travel_delay = 0;
    info.max_travel_delay = 0;

    for (unsigned int d = 0; d < info.max_departure_delay_; d++) {
      generate_distribution(info.distributions[d], d, t);
      if (info.distributions[d].first_minute() < info.min_travel_delay)
        info.min_travel_delay = info.distributions[d].first_minute();
      if (info.distributions[d].last_minute() > info.max_travel_delay)
        info.max_travel_delay = info.distributions[d].last_minute();
    }
  }
}

void tt_distributions_manager::generate_distribution(
    probability_distribution& distribution, unsigned int const depature_delay,
    unsigned int const travel_duration) {
  distribution.init_one_point(0, 1.0);  // TODO
}

}  // namespace reliability
}  // namespace motis
