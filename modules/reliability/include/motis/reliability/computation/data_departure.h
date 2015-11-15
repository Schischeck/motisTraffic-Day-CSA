#pragma once

#include <vector>

#include "motis/core/schedule/time.h"

#include "motis/reliability/graph_accessor.h"

namespace motis {
struct category;
struct light_connection;
class node;
struct schedule;

namespace reliability {
struct probability_distribution;
struct start_and_travel_distributions;

namespace distributions_container {
struct abstract_distributions_container;
}

namespace calc_departure_distribution {

/**
 * Class storing all data necessary for calculating a departure distribution.
 */
struct data_departure {
  /* todo: use reliability::context */
  data_departure(
      node const& route_node, light_connection const& light_connection,
      bool const is_first_route_node, schedule const& schedule,
      distributions_container::abstract_distributions_container const&
          train_distributions_container,
      distributions_container::abstract_distributions_container const&
          feeder_distributions_container,
      start_and_travel_distributions const& s_t_distributions);

  /* constructor necessary for the derived struct data_departure_connection */
  data_departure(bool const is_first_route_node, time const scheduled_dep_time);

  duration largest_delay() const;

  void debug_output(std::ostream& os) const;

  bool const is_first_route_node_;

  time const scheduled_departure_time_;

  union train_info {
    struct preceding_arrival_info {
      /** scheduled arrival time of the preceding arrival event of the train */
      time arrival_time_;

      /** arrival distribution of the preceding arrival event of the train */
      probability_distribution const* arrival_distribution_;

      /** minimum standing time of the train at the station */
      duration min_standing_;
    } preceding_arrival_info_;

    probability_distribution const* first_departure_distribution_;
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

  /** arrival distributions of the feeder trains
   * TODO: do not store the interchange feeder in this vector!
   * (distributions_calculator makes this assumption).
   */
  std::vector<feeder_info> feeders_;
  /** maximal waiting time of the departing train of any feeder */
  duration maximum_waiting_time_;

protected:
  void init_train_info(
      node const& route_node, light_connection const& light_conn,
      distributions_container::abstract_distributions_container const&
          distributions_container,
      start_and_travel_distributions const& s_t_distributions,
      std::vector<std::unique_ptr<category>> const& categories);

  void init_feeder_info(
      light_connection const& light_conn,
      std::vector<std::unique_ptr<graph_accessor::feeder_info>> const&
          all_feeders_data,
      schedule const& schedule,
      distributions_container::abstract_distributions_container const&
          distributions_container);
};

}  // namespace calc_departure_distribution
}  // namespace reliability
}  // namespace motis
