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
struct train_distributions_container;
struct travel_distribution_info;
struct tt_distributions_manager;

/**
 * struct storing all data necessary for calculating an arrival distribution.
 */
struct pd_calc_data_arrival {
  pd_calc_data_arrival(
      node const& route_node, light_connection const& light_connection,
      schedule const& schedule,
      train_distributions_container const& distributions_container,
      tt_distributions_manager const& tt_dist_manager);

  time get_scheduled_arrival_time_() const;
  duration get_scheduled_travel_duration() const;

  void debug_output(std::ostream& os) const;


  node const& route_node_;  // XXX is required?

  light_connection const& light_connection_;  // XXX is required?

  struct departure_info {
    probability_distribution const* distribution_;
    time scheduled_departure_time_;
  } departure_info_;

  travel_distribution_info const* travel_time_info_;

private:
  void init_departure_info(
      train_distributions_container const& distributions_container);
  void init_travel_time_info(std::vector<std::string> const& category_names,
                             tt_distributions_manager const& tt_dist_manager);
};

}  // namespace reliability
}  // namespace motis
