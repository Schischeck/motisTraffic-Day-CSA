#include "motis/reliability/computation/data_departure_interchange.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/distributions/probability_distribution.h"
#include "motis/reliability/distributions/start_and_travel_distributions.h"
#include "motis/reliability/realtime/time_util.h"

#include <cassert>

namespace motis {
namespace reliability {
namespace calc_departure_distribution {

/** get vector containing all potential feeders but not arriving_light_conn */
std::vector<graph_accessor::feeder_info> get_all_potential_feeders_except_ic(
    node const& route_node, light_connection const& departing_light_conn,
    light_connection const& arriving_light_conn, duration const transfer_time) {
  auto all_feeders = graph_accessor::get_all_potential_feeders(
      route_node, departing_light_conn.d_time, transfer_time);
  /* todo: accelerate with lower_bound? */
  auto const it = std::find_if(
      all_feeders.begin(), all_feeders.end(),
      [arriving_light_conn](graph_accessor::feeder_info const& feeder) {
        return feeder.light_conn_->d_time == arriving_light_conn.d_time &&
               feeder.light_conn_->a_time == arriving_light_conn.a_time &&
               feeder.light_conn_->_full_con->con_info ==
                   arriving_light_conn._full_con->con_info;
      });
  if (it != all_feeders.end()) {
    all_feeders.erase(it);
  }
  return all_feeders;
}

data_departure_interchange::data_departure_interchange(
    bool const is_first_route_node, node const& route_node,
    light_connection const& departing_light_conn,
    light_connection const& arriving_light_conn,
    probability_distribution const& arrival_distribution,
    distributions_container::container const& distribution_preceding_train,
    reliability::context const& context)
    : data_departure(is_first_route_node,
                     time_util::get_scheduled_event_time(
                         departing_light_conn, route_node.get_station()->_id,
                         time_util::departure, context.schedule_)) {
  if (is_first_route_node) {
    init_first_departure_info(departing_light_conn, context.s_t_distributions_,
                              context.schedule_.categories);
  } else {
    init_preceding_arrival_info(route_node, departing_light_conn.d_time,
                                distribution_preceding_train,
                                context.schedule_);
  }

  duration const transfer_time =
      context.schedule_.stations[route_node._station_node->_id]
          ->transfer_time; /* TODO: use track change time if possible) */
  duration const waiting_time = graph_accessor::get_waiting_time(
      context.schedule_.waiting_time_rules_, arriving_light_conn,
      departing_light_conn);
  init_interchange_feeder_info(
      time_util::get_scheduled_event_time(
          arriving_light_conn, route_node.get_station()->_id,
          time_util::arrival, context.schedule_),
      arrival_distribution, transfer_time, waiting_time);

  auto const all_feeders_data = get_all_potential_feeders_except_ic(
      route_node, departing_light_conn, arriving_light_conn,
      context.schedule_.stations[route_node._station_node->_id]->transfer_time);
  init_feeder_info(departing_light_conn, route_node.get_station()->_id,
                   all_feeders_data, context.schedule_,
                   context.precomputed_distributions_);
  maximum_waiting_time_ =
      std::max(maximum_waiting_time_, interchange_feeder_info_.waiting_time_);
}

void data_departure_interchange::init_interchange_feeder_info(
    time const scheduled_arrival_time,
    probability_distribution const& arrival_distribution,
    duration const transfer_time, duration const waiting_time) {
  interchange_feeder_info_.scheduled_arrival_time_ = scheduled_arrival_time;
  interchange_feeder_info_.arrival_distribution_ = &arrival_distribution;
  interchange_feeder_info_.transfer_time_ = transfer_time;
  interchange_feeder_info_.waiting_time_ = waiting_time;
  interchange_feeder_info_.latest_feasible_arrival_ =
      (scheduled_departure_time_ - interchange_feeder_info_.transfer_time_) +
      interchange_feeder_info_.waiting_time_;

  assert(interchange_feeder_info_.scheduled_arrival_time_ <=
         scheduled_departure_time_);
  assert(!interchange_feeder_info_.arrival_distribution_->empty());
}

data_departure_interchange::data_departure_interchange(
    node const& route_node, light_connection const& light_connection,
    bool const is_first_route_node,
    distributions_container::container const& preceding_arrival_distribution,
    reliability::context const& context)
    : data_departure(route_node, light_connection, is_first_route_node,
                     preceding_arrival_distribution, context) {
  // nothing to do
}

data_departure_interchange_walk::data_departure_interchange_walk(
    bool const is_first_route_node, node const& departing_route_node,
    node const& arrival_station, light_connection const& departing_light_conn,
    light_connection const& arriving_light_conn,
    probability_distribution const& arrival_distribution,
    distributions_container::container const& preceding_arrival_distribution,
    reliability::context const& context)
    : data_departure_interchange(departing_route_node, departing_light_conn,
                                 is_first_route_node,
                                 preceding_arrival_distribution, context) {
  init_interchange_feeder_info(
      time_util::get_scheduled_event_time(
          arriving_light_conn, arrival_station.get_station()->_id,
          time_util::arrival, context.schedule_),
      arrival_distribution,
      graph_accessor::walking_duration(arrival_station,
                                       *departing_route_node._station_node),
      0);
}

}  // namespace calc_departure_distribution
}  // namespace reliability
}  // namespace motis
