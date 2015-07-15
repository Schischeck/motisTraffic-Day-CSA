#pragma once

#include <iostream>
#include <vector>

#include "motis/core/schedule/time.h"

namespace motis {

class light_connection;
class node;
struct schedule;

namespace reliability {

struct probability_distribution;
struct tt_distributions_manager;

/**
 * struct storing all data necessary for calculating an arrival distribution.
 */
struct pd_calc_data_arrival {
  pd_calc_data_arrival(tt_distributions_manager const& distributions_manager,
                       node& route_node,
                       light_connection const& light_connection,
                       schedule const& schedule);

  duration get_scheduled_travel_duration() const {
    return scheduled_arrival_time_ - departure_info_.scheduled_departure_time_;
  }

  probability_distribution const& get_travel_time_distribution(
      unsigned int delay) const;

  void debug_output(std::ostream& os) const;

  node& route_node_;  // XXX is required?

  light_connection const& light_connection_;  // XXX is required?

  struct departure_info {
    probability_distribution const* distribution_;
    time scheduled_departure_time_;
  } departure_info_;

  time scheduled_arrival_time_;

  struct travel_time_info {
    friend struct pd_calc_data_arrival;

    probability_distribution const& get_travel_time_distribution(
        unsigned int departure_delay) const;

    /** minimal travel delay. Usually this is a negative value
     * and corresponds to the maximal catch up potential. */
    int min_travel_delay_;

    int max_travel_delay_;

    unsigned int max_departure_delay_;

  private:
    /** a travel-time distribution for each delay value */
    std::vector<probability_distribution> travel_time_distributions;
  } travel_time_info_;

private:
  void init_generated_tt_distribution(
      tt_distributions_manager const& distributions_manager);
};

}  // namespace reliability
}  // namespace motis
