#pragma once

#include <iostream>
#include <vector>

#include "motis/core/schedule/time.h"

#include "motis/reliability/start_and_travel_distributions.h"

namespace motis {

class light_connection;
class node;
struct schedule;

namespace reliability {

struct probability_distribution;
struct train_distributions_container;

/**
 * struct storing all data necessary for calculating an arrival distribution.
 */
struct pd_calc_data_arrival {
  pd_calc_data_arrival(node const& route_node,
                       light_connection const& light_connection,
                       probability_distribution const& departure_distribution,
                       schedule const& schedule,
                       start_and_travel_distributions const& s_t_distributions);

  time scheduled_arrival_time_() const;
  duration scheduled_travel_duration() const;

  void debug_output(std::ostream& os) const;

  node const& route_node_;  // XXX is required?

  light_connection const& light_connection_;  // XXX is required?

  struct departure_info {
    departure_info(probability_distribution const& distribution,
                   time const scheduled_departure_time)
        : distribution_(distribution),
          scheduled_departure_time_(scheduled_departure_time) {}
    probability_distribution const& distribution_;
    time const scheduled_departure_time_;
  } departure_info_;

  std::vector<start_and_travel_distributions::probability_distribution_cref>
      travel_distributions_;

  /* bounds of the arrival distribution */
  int left_bound_;
  int right_bound_;

private:
  void init_travel_info(start_and_travel_distributions const& s_t_distributions,
                        std::vector<std::string> const& category_names);
};

}  // namespace reliability
}  // namespace motis
