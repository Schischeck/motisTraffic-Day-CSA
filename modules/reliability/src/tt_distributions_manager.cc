#include "motis/reliability/tt_distributions_manager.h"
#include "motis/reliability/probability_distribution.h"

namespace motis {
namespace reliability {

tt_distributions_manager::tt_distributions_manager() {
  init_generated_distributions();
}

std::tuple<std::vector<probability_distribution> const&, int, int>
tt_distributions_manager::get_travel_time_distributions(
    std::string const& train_category, duration const travel_time) const {
  return std::make_tuple(generated_distributions_info_.distributions,
                         generated_distributions_info_.min_travel_delay,
                         generated_distributions_info_.max_travel_delay);
}

void tt_distributions_manager::init_generated_distributions() {
  generated_distributions.resize(longest_travel_time_);

  // for each train-edge length there exist another distribution
  for (unsigned int t = 0; t < longest_travel_time_; t++) {
    auto& info = generated_distributions[t];
    info.distributions.resize(maximum_departure_delay_);
    info.min_travel_delay = 0;
    info.max_travel_delay = 0;

    for (unsigned int d = 0; d < maximum_departure_delay_; d++) {
      generate_distribution(info.distributions[d], d, t);
      if (info.distributions[d].get_first_minute() < info.min_travel_delay)
        info.min_travel_delay = info.distributions[d].get_first_minute();
      if (info.distributions[d].get_last_minute() > info.max_travel_delay)
        info.max_travel_delay = info.distributions[d].get_last_minute();
    }
  }
}

void tt_distributions_manager::generate_distribution(
    probability_distribution& distribution, unsigned int const depature_delay,
    unsigned int const travel_duration) {
  distribution.init_one_point(0);  // TODO
}

}  // namespace reliability
}  // namespace motis
