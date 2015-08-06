#pragma once

#include <functional>
#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "motis/core/schedule/time.h"

#include "motis/reliability/probability_distribution.h"

namespace motis {
namespace reliability {

struct travel_distribution_info;

// XXX two separated classes for start and travel-time distributions?!
struct tt_distributions_manager {

  tt_distributions_manager();
  virtual ~tt_distributions_manager() {}
  void initialize();

  virtual probability_distribution const& get_start_distribution(
      std::string const& train_category) const;

  virtual travel_distribution_info const& get_travel_time_distributions(
      std::string const& train_category, duration const travel_time) const;

  duration const longest_travel_time_ = 300;

protected:
  void init_distribution_classes();
  void init_start_distributions();
  void init_generated_distributions();

  void generate_distribution(probability_distribution& distribution,
                             unsigned int const depature_delay,
                             unsigned int const travel_duration);

  /** Mapping from train category (ICE, IC, ...)
   * to distribution class (RV, FV, ...) */
  std::map<std::string, std::string> train_category_to_distribution_class_;

  /** Mapping from train class (RV, FV, ...) to the start distributions */
  std::map<std::string, probability_distribution>
      distribution_class_to_start_distribution_;
  probability_distribution default_start_distribution_;

  std::vector<travel_distribution_info> travel_time_to_generated_distribution_;
};

struct travel_distribution_info {
  travel_distribution_info(
      std::vector<probability_distribution> const& distributions,
      unsigned int const max_departure_delay, int const min_travel_delay,
      int const max_travel_delay)
      : distributions_(distributions),
        max_departure_delay_(max_departure_delay),
        min_travel_delay_(min_travel_delay),
        max_travel_delay_(max_travel_delay) {}

  /** empty constructor required for vectors */
  travel_distribution_info()
      : max_departure_delay_(0),
        min_travel_delay_(0),
        max_travel_delay_(0) {}

  probability_distribution const& get_travel_time_distribution(
      unsigned int departure_delay) const {
    return distributions_[std::min(departure_delay, max_departure_delay_ - 1)];
  }

  unsigned int const max_departure_delay_;
  /** minimum and maximum allowed travel delay of the
   * travel time distribution depending on the scheduled travel time */
  int const min_travel_delay_;
  int const max_travel_delay_;

private:
  /* travel-time distributions indexed by departure delay */
  std::vector<probability_distribution> const distributions_;
};

}  // namespace reliability
}  // namespace motis
