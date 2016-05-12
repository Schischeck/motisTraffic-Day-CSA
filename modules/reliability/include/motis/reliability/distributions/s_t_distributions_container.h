#pragma once

#include "db_distributions.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "motis/reliability/distributions/db_distributions_loader.h"

namespace motis {
namespace reliability {

struct probability_distribution;

struct s_t_distributions_container : start_and_travel_distributions {
  struct parameters {
    std::string root_;
    unsigned int max_expected_travel_time_;
    unsigned int max_expected_departure_delay_;
  };
  explicit s_t_distributions_container(std::vector<parameters> const&);

  std::pair<bool, probability_distribution_cref> get_start_distribution(
      std::string const& family) const override;

  bool get_travel_time_distributions(
      std::string const& family, unsigned int const travel_time,
      unsigned int const to_departure_delay,
      std::vector<probability_distribution_cref>& distributions) const override;

private:
  std::vector<std::unique_ptr<db_distributions>> distributions_;
  probability_distribution dummy_distribution_;
};

}  // namespace reliability
}  // namespace motis
