#pragma once

#include <memory>
#include <vector>

#include "motis/core/schedule/time.h"

namespace motis {

class light_connection;
class node;
struct schedule;

namespace reliability {

struct probability_distribution;
class train_distributions;
struct train_distributions_container;
class tt_distributions_manager;

/**
 * Class storing all data necessary for calculating a departure distribution.
 */
struct pd_calc_data_departure {

  pd_calc_data_departure(
      node const& route_node, light_connection const& light_connection,
      bool const is_first_route_node, schedule const& schedule,
      train_distributions_container const& distributions_container,
      tt_distributions_manager const& tt_dist_manager);

  time scheduled_departure_time() const;

  duration largest_delay(void) const;

  void debug_output(std::ostream& os) const;

  node const& route_node_;  // XXX is required?

  light_connection const& light_connection_;  // XXX is required?

  bool const is_first_route_node_;

  union train_info {
    struct preceding_arrival_info {
      /** scheduled arrival time of the preceding arrival event of the train
       * (0 if this is the first departure) */
      time arrival_time_;

      /** arrival distribution of the preceding arrival event of the train
       * (Null if this is the first departure) */
      probability_distribution const* arrival_distribution_;

      /** minimum standing time of the train at the station */
      duration min_standing_;
    } preceding_arrival_info_;

    probability_distribution const* first_departure_distribution;
  } train_info_;

  struct feeder_info {
    feeder_info(probability_distribution const& distribution,
                time const arrival_time, time const latest_feasible_arrival,
                duration const transfer_time)
        : distribution_(distribution),
          arrival_time_(arrival_time),
          latest_feasible_arrival_(latest_feasible_arrival),
          transfer_time_(transfer_time) {}

    probability_distribution const& distribution_;
    time const arrival_time_;
    /** latest arrival times of the feeder such that an interchange
     * from the feeder into the departing train is possible */
    time const latest_feasible_arrival_;
    /** required times for an interchange from a feeder into the train */
    duration const transfer_time_;
  };

  /** arrival distributions of the feeder trains */
  std::vector<feeder_info> feeders_;

  /** maximal waiting time of the departing train of any feeder */
  duration maximum_waiting_time_;

private:
  void init_train_info(
      std::vector<std::string> const& category_names,
      tt_distributions_manager const& tt_dist_manager,
      train_distributions_container const& distributions_container);

  void init_feeder_info(
      schedule const& schedule,
      train_distributions_container const& distributions_container);
};

}  // namespace reliability
}  // namespace motis
