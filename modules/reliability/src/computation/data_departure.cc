#include "motis/reliability/computation/data_departure.h"

#include <algorithm>

#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/waiting_time_rules.h"

#include "motis/reliability/distributions/distributions_container.h"
#include "motis/reliability/distributions/probability_distribution.h"
#include "motis/reliability/distributions/start_and_travel_distributions.h"
#include "motis/reliability/graph_accessor.h"

namespace motis {
namespace reliability {
namespace calc_departure_distribution {
data_departure::data_departure(node const& route_node,
                               light_connection const& light_connection,
                               motis::time const scheduled_departure_time,
                               reliability::context const& context)
    : is_first_route_node_(true),
      scheduled_departure_time_(scheduled_departure_time),
      maximum_waiting_time_(0) {
  init_first_departure_info(light_connection, context.s_t_distributions_,
                            context.schedule_.categories);
  auto const all_feeders_data = graph_accessor::get_all_potential_feeders(
      route_node, light_connection,
      context.schedule_.stations[route_node._station_node->_id]->transfer_time);
  init_feeder_info(light_connection, all_feeders_data, context.schedule_,
                   context.precomputed_distributions_);
}

data_departure::data_departure(
    node const& route_node, light_connection const& light_connection,
    motis::time const scheduled_departure_time,
    reliability::context const& context,
    motis::time const scheduled_arrival_time,
    distributions_container::container const& train_distributions_container)
    : is_first_route_node_(false),
      scheduled_departure_time_(scheduled_departure_time),
      maximum_waiting_time_(0) {
  init_preceding_arrival_info(route_node, light_connection.d_time,
                              scheduled_arrival_time,
                              train_distributions_container);
  auto const all_feeders_data = graph_accessor::get_all_potential_feeders(
      route_node, light_connection,
      context.schedule_.stations[route_node._station_node->_id]->transfer_time);
  init_feeder_info(light_connection, all_feeders_data, context.schedule_,
                   context.precomputed_distributions_);
}

data_departure::data_departure(bool const is_first_route_node,
                               time const scheduled_dep_time)
    : is_first_route_node_(is_first_route_node),
      scheduled_departure_time_(scheduled_dep_time),
      maximum_waiting_time_(0) {}

void data_departure::init_first_departure_info(
    light_connection const& light_conn,
    start_and_travel_distributions const& s_t_distributions,
    std::vector<std::unique_ptr<category>> const& categories) {
  auto const& train_category =
      categories[light_conn._full_con->con_info->family]->name;
  train_info_.first_departure_distribution_ =
      &s_t_distributions.get_start_distribution(train_category);
  assert(!train_info_.first_departure_distribution_->empty());
}

void data_departure::init_preceding_arrival_info(
    node const& route_node, motis::time const departure_time,
    motis::time const scheduled_arrival_time,
    distributions_container::container const& distributions_container) {
  auto const& arriving_light_conn =
      graph_accessor::get_previous_light_connection(
          graph_accessor::get_arriving_route_edge(route_node)
              ->_m._route_edge._conns,
          departure_time);

  train_info_.preceding_arrival_info_.arrival_time_ =
      arriving_light_conn.a_time;
  train_info_.preceding_arrival_info_.arrival_distribution_ =
      &distributions_container.get_distribution(
          distributions_container::to_container_key(
              arriving_light_conn, route_node.get_station()->_id,
              distributions_container::container::key::arrival,
              scheduled_arrival_time));
  // the standing-time is always less or equal 2 minutes
  train_info_.preceding_arrival_info_.min_standing_ =
      std::min(2, scheduled_departure_time_ - arriving_light_conn.a_time);

  assert(train_info_.preceding_arrival_info_.arrival_time_ <=
         scheduled_departure_time_);
  assert(!train_info_.preceding_arrival_info_.arrival_distribution_->empty());
}

void data_departure::init_feeder_info(
    light_connection const& light_conn, unsigned int const station_id,
    std::vector<light_connection const*> const& feeders,
    schedule const& schedule,
    distributions_container::abstract_distributions_container const&
        distributions_container) {
  for (auto const& feeder : feeders) {
    auto waiting_time = graph_accessor::get_waiting_time(
        schedule.waiting_time_rules_, *feeder, light_conn);
    if (waiting_time > 0) {
      auto const transfer_time = schedule.stations[station_id]->transfer_time;
      time const latest_feasible_arrival =
          (scheduled_departure_time_ + waiting_time) - transfer_time;
      auto const& feeder_distribution =
          distributions_container.get_distribution(
              distributions_container::to_container_key(
                  *feeder, station_id,
                  distributions_container::container::key::arrival,
                  feeder->scheduled_arrival_time));

      feeders_.emplace_back(feeder_distribution, feeder->light_conn_.a_time,
                            latest_feasible_arrival, transfer_time);

      if (maximum_waiting_time_ < waiting_time) {
        maximum_waiting_time_ = waiting_time;
      }
    }
  }  // end of for all_feeders_data
}

duration data_departure::largest_delay() const {
  duration maximum_train_delay = 0;
  if (is_first_route_node_) {
    maximum_train_delay =
        (duration)train_info_.first_departure_distribution_->last_minute();
  } else {
    time const latest_arrival =
        train_info_.preceding_arrival_info_.arrival_time_ +
        train_info_.preceding_arrival_info_.arrival_distribution_
            ->last_minute() +
        train_info_.preceding_arrival_info_.min_standing_;
    maximum_train_delay = latest_arrival < scheduled_departure_time_
                              ? 0
                              : latest_arrival - scheduled_departure_time_;
  }
  return std::max(maximum_train_delay, maximum_waiting_time_);
}

void data_departure::debug_output(std::ostream& os) const {
  os << "pd_calc_data_departure:\n"
     << "scheduled-departure-time: " << format_time(scheduled_departure_time_)
     << " largest-delay: " << largest_delay()
     << " is-first-route-node: " << is_first_route_node_;

  if (is_first_route_node_) {
    os << "\nstart-distribution: "
       << *train_info_.first_departure_distribution_;
  } else {
    os << "\npreceding-arrival-time: "
       << format_time(train_info_.preceding_arrival_info_.arrival_time_)
       << " min-standing: " << train_info_.preceding_arrival_info_.min_standing_
       << "\npreceding-arrival-distribution: "
       << *train_info_.preceding_arrival_info_.arrival_distribution_;
  }

  os << "\nFeeders:";

  for (auto const& feeder : feeders_) {
    os << "\nfeeder-arrival-time: " << format_time(feeder.arrival_time_)
       << " lfa: " << feeder.latest_feasible_arrival_
       << " transfer-time: " << feeder.transfer_time_
       << "\nfeeder-distribution: " << feeder.distribution_ << "\n";
  }

  os << "maximum-waiting-time: " << maximum_waiting_time_;
  os << std::endl;
}

}  // namespace calc_departure_distribution
}  // namespace reliability
}  // namespace motis
