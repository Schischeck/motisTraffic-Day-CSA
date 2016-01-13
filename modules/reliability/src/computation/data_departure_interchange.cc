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
std::vector<distributions_container::container::node*>
get_all_potential_feeders_except_ic(
    std::vector<distributions_container::container::node*> const& feeders,
    distributions_container::container::key const& ic_feeder) {
  std::vector<distributions_container::container::node*> filtered_feeders(
      feeders);
  filtered_feeders.erase(
      std::remove_if(
          filtered_feeders.begin(), filtered_feeders.end(),
          [&](distributions_container::container::node const* feeder) {
            return feeder->key_ == ic_feeder;
          }),
      filtered_feeders.end());
  return filtered_feeders;
}

data_departure_interchange::data_departure_interchange(
    bool const is_first_route_node, node const& departing_route_node,
    node const& arriving_route_node,
    light_connection const& departing_light_conn,
    light_connection const& arriving_light_conn,
    probability_distribution const& arrival_distribution,
    distributions_container::container const& distribution_preceding_train,
    distributions_container::container::node const& departing_distribution_node,
    reliability::context const& context)
    : data_departure(departing_route_node, departing_light_conn,
                     is_first_route_node, context.schedule_) {
  if (is_first_route_node) {
    init_first_departure_info(departing_light_conn, context.s_t_distributions_,
                              context.schedule_.categories);
  } else {
    init_preceding_arrival_info(
        departing_route_node, departing_light_conn.d_time,
        distribution_preceding_train, context.schedule_);
  }

  duration const transfer_time = graph_accessor::get_interchange_time(
      arriving_route_node, departing_route_node, context.schedule_);
  duration const waiting_time = graph_accessor::get_waiting_time(
      context.schedule_.waiting_time_rules_, arriving_light_conn,
      departing_light_conn);

  init_interchange_feeder_info(time_util::get_scheduled_event_time(
                                   arriving_route_node, arriving_light_conn,
                                   time_util::arrival, context.schedule_),
                               arrival_distribution, transfer_time,
                               waiting_time);

  auto const all_feeders_data = get_all_potential_feeders_except_ic(
      departing_distribution_node.predecessors_,
      distributions_container::to_container_key(
          arriving_route_node, arriving_light_conn, time_util::arrival,
          context.schedule_));
  init_feeder_info(departing_route_node, departing_light_conn, all_feeders_data,
                   context.schedule_);
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
    distributions_container::container::node const& departing_distribution_node,
    reliability::context const& context)
    : data_departure(route_node, light_connection, is_first_route_node,
                     preceding_arrival_distribution,
                     departing_distribution_node, context) {
  // nothing to do
}

data_departure_interchange_walk::data_departure_interchange_walk(
    bool const is_first_route_node, node const& departing_route_node,
    node const& arriving_route_node,
    light_connection const& departing_light_conn,
    light_connection const& arriving_light_conn,
    probability_distribution const& arrival_distribution,
    distributions_container::container const& preceding_arrival_distribution,
    distributions_container::container::node const& departing_distribution_node,
    reliability::context const& context)
    : data_departure_interchange(departing_route_node, departing_light_conn,
                                 is_first_route_node,
                                 preceding_arrival_distribution,
                                 departing_distribution_node, context) {
  init_interchange_feeder_info(
      time_util::get_scheduled_event_time(
          arriving_route_node, arriving_light_conn, time_util::arrival,
          context.schedule_),
      arrival_distribution,
      graph_accessor::walking_duration(*arriving_route_node.get_station(),
                                       *departing_route_node.get_station()),
      0);
}

}  // namespace calc_departure_distribution
}  // namespace reliability
}  // namespace motis
