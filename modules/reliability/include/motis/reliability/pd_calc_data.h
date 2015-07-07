#pragma once

#include <memory>
#include <vector>

#include "motis/core/schedule/Time.h"

namespace td {
class LightConnection;
class Node;
struct Schedule;
}

namespace motis {
namespace reliability {

struct probability_distribution;
class train_distributions;
struct train_distributions_container;
class tt_distribution_manager;

/**
 * Class storing all data necessary for calculating a departure distribution.
 */
struct pd_calc_data_departure {

  struct feeder_info {
    feeder_info(probability_distribution const& distribution,
                td::Time const arrival_time,
                td::Time const latest_feasible_arrival,
                td::Duration const transfer_time)
        : distribution_(distribution),
          arrival_time_(arrival_time),
          latest_feasible_arrival_(latest_feasible_arrival),
          transfer_time_(transfer_time) {}

    probability_distribution const& distribution_;
    td::Time const arrival_time_;
    /** latest arrival times of the feeder such that an interchange
     * from the feeder into the departing train is possible */
    td::Time const latest_feasible_arrival_;
    /** required times for an interchange from a feeder into the train */
    td::Duration const transfer_time_;
  };

  pd_calc_data_departure(
      td::Node& route_node, td::LightConnection const& light_connection,
      bool const is_first_route_node, td::Schedule const& schedule,
      tt_distribution_manager const& tt_dist_manager,
      train_distributions_container& distributions_container);

  td::Duration get_largest_delay(void) const;

  void debug_output(std::ostream& os) const;

  td::Node& route_node_;  // XXX is required?

  td::LightConnection const& light_connection_;  // XXX is required?

  /** arrival distributions of the feeder trains */
  std::vector<feeder_info> feeders_;

  /** maximal waiting time of the departing train of any feeder */
  td::Duration maximum_waiting_time_;

  bool const is_first_route_node_;

  union train_info {
    struct preceding_arrival_info {
      /** scheduled arrival time of the preceding arrival event of the train
       * (0 if this is the first departure) */
      td::Time arrival_time_;

      /** arrival distribution of the preceding arrival event of the train
       * (Null if this is the first departure) */
      probability_distribution const* arrival_distribution_;

      /** minimum standing time of the train at the station */
      td::Duration min_standing_;
    } preceding_arrival_info_;

    probability_distribution const* first_departure_distribution;
  } train_info_;

private:
  void init_train_info(std::vector<std::string> const& category_names,
                       tt_distribution_manager const& tt_dist_manager,
                       std::vector<std::unique_ptr<train_distributions> > const&
                           node_to_train_distributions);

  void init_feeder_info(
      td::Schedule const& schedule,
      std::vector<std::unique_ptr<train_distributions> > const&
          node_to_train_distributions);
};

}  // namespace reliability
}  // namespace motis
