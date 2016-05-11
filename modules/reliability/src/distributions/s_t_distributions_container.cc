#include "motis/reliability/distributions/s_t_distributions_container.h"

#include "motis/core/common/logging.h"

namespace motis {
namespace reliability {

s_t_distributions_container::s_t_distributions_container(
    std::vector<parameters> const& param) {
  for (auto const& p : param) {
    distributions_.emplace_back(new db_distributions(
        p.root_, p.max_expected_travel_time_, p.max_expected_departure_delay_));
  }
  dummy_distribution_.init_one_point(0, 1.0);
}

std::pair<bool, start_and_travel_distributions::probability_distribution_cref>
s_t_distributions_container::get_start_distribution(
    std::string const& family) const {
  for (auto const& d : distributions_) {
    auto const& pd = d->get_start_distribution(family);
    if (pd.first) {
      return pd;
    }
  }
  return std::make_pair(false, std::cref(dummy_distribution_));
}

bool s_t_distributions_container::get_travel_time_distributions(
    std::string const& family, unsigned int const travel_time,
    unsigned int const to_departure_delay,
    std::vector<probability_distribution_cref>& distributions) const {
  for (auto const& d : distributions_) {
    distributions.clear();
    auto const success = d->get_travel_time_distributions(
        family, travel_time, to_departure_delay, distributions);
    if (success) {
      return true;
    }
  }
  return false;
}

}  // namespace reliability
}  // namespace motis
