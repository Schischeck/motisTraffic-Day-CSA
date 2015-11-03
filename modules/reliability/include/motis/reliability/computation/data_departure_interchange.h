#pragma once

#include "motis/reliability/computation/data_departure.h"

namespace motis {
class light_connection;
class node;
struct schedule;

namespace reliability {
struct probability_distribution;
struct start_and_travel_distributions;

namespace distributions_container {
struct abstract_distributions_container;
}

namespace graph_accessor {
struct feeder_info;
}

namespace calc_departure_distribution {

/** arrival and departure at the same station (no walking) */
struct data_departure_interchange : data_departure {
  data_departure_interchange(
      bool const is_first_route_node, node const& route_node,
      light_connection const& departing_light_conn,
      light_connection const& arriving_light_conn,
      probability_distribution const& arrival_distribution,
      schedule const& schedule,
      distributions_container::abstract_distributions_container const&
          train_distributions_container,
      distributions_container::abstract_distributions_container const&
          feeder_distributions_container,
      start_and_travel_distributions const& s_t_distributions);

  struct interchange_feeder_info {
    time scheduled_arrival_time_;
    probability_distribution const* arrival_distribution_;
    duration transfer_time_;
    duration waiting_time_;
    duration latest_feasible_arrival_;
  } interchange_feeder_info_;

protected:
  /** constructor required by data_departure_interchange_walk */
  data_departure_interchange(
      node const& route_node, light_connection const& light_connection,
      bool const is_first_route_node, schedule const& schedule,
      distributions_container::abstract_distributions_container const&
          train_distributions_container,
      distributions_container::abstract_distributions_container const&
          feeder_distributions_container,
      start_and_travel_distributions const& s_t_distributions);

  void init_interchange_feeder_info(
      time const scheduled_arrival_time,
      probability_distribution const& arrival_distribution,
      duration const transfer_time, duration const waiting_time);
};

/* arrival and departure at different stations (walking)
 * Note: is_first_route_node: departing_route_node is the first route node
 * of the departing train */
struct data_departure_interchange_walk : data_departure_interchange {
  data_departure_interchange_walk(
      bool const is_first_route_node, node const& departing_route_node,
      node const& arrival_station, light_connection const& departing_light_conn,
      light_connection const& arriving_light_conn,
      probability_distribution const& arrival_distribution,
      schedule const& schedule,
      distributions_container::abstract_distributions_container const&
          train_distributions_container,
      distributions_container::abstract_distributions_container const&
          feeder_distributions_container,
      start_and_travel_distributions const& s_t_distributions);
};

}  // namespace calc_departure_distribution
}  // namespace reliability
}  // namespace motis
