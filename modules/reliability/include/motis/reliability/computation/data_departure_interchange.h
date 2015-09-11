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
struct precomputed_distributions_container;
}

namespace graph_accessor {
struct feeder_info;
}

namespace calc_departure_distribution {

/**
 * Class storing all data necessary for calculating a departure distribution.
 */
struct data_departure_interchange : data_departure {
  data_departure_interchange(
      bool const is_first_route_node, node const& route_node,
      light_connection const& departing_light_conn,
      light_connection const& arriving_light_conn,
      probability_distribution const& arrival_distribution,
      schedule const& schedule,
      distributions_container::precomputed_distributions_container const&
          precomputed_distributions,
      start_and_travel_distributions const& s_t_distributions);

  struct interchange_feeder_info {
    time scheduled_arrival_time_;
    probability_distribution const* arrival_distribution_;
    duration transfer_time_;
    duration waiting_time_;
  } interchange_feeder_info_;

private:
  void init_interchange_feeder_info(
      node const& route_node, light_connection const& arriving_light_conn,
      probability_distribution const& arrival_distribution,
      light_connection const& departing_light_conn, schedule const& schedule);
};

}  // namespace calc_departure_distribution
}  // namespace reliability
}  // namespace motis
