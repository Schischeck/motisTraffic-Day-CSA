#include "motis/reliability/pd_calc_data_departure.h"

#include <algorithm>

#include "../include/motis/reliability/tt_distributions_manager.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/waiting_time_rules.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/probability_distribution.h"
#include "motis/reliability/train_distributions.h"

namespace motis {
namespace reliability {

pd_calc_data_departure::pd_calc_data_departure(
    node& route_node, light_connection const& light_connection,
    bool const is_first_route_node, schedule const& schedule,
    tt_distributions_manager const& tt_dist_manager,
    train_distributions_container& distributions_container)
    : route_node_(route_node),
      light_connection_(light_connection),
      is_first_route_node_(is_first_route_node),
      maximum_waiting_time_(0) {

  init_train_info(schedule.category_names, tt_dist_manager,
                  distributions_container.node_to_train_distributions_);
  init_feeder_info(schedule,
                   distributions_container.node_to_train_distributions_);
}

void pd_calc_data_departure::init_train_info(
    std::vector<std::string> const& category_names,
    tt_distributions_manager const& tt_dist_manager,
    std::vector<std::unique_ptr<train_distributions> > const&
        node_to_train_distributions) {
  if (is_first_route_node_) {
    auto const& train_category =
        category_names[light_connection_._full_con->con_info->family];
    train_info_.first_departure_distribution =
        &tt_dist_manager.get_start_distribution(train_category);
  } else {
    light_connection const* arriving_light_conn;
    unsigned int distribution_pos;
    std::tie(arriving_light_conn, distribution_pos) =
        graph_accessor::get_previous_light_connection(route_node_,
                                                      light_connection_);

    train_info_.preceding_arrival_info_.arrival_time_ =
        arriving_light_conn->a_time;
    train_info_.preceding_arrival_info_.arrival_distribution_ =
        &node_to_train_distributions[route_node_._id]
             ->arrival_distributions_[distribution_pos];

    // the standing-time is always less or equal 2 minutes
    train_info_.preceding_arrival_info_.min_standing_ =
        std::min(2, light_connection_.d_time - arriving_light_conn->a_time);
  }
}

inline duration get_waiting_time(
    waiting_time_rules const& waiting_time_rules,
    light_connection const& feeder_light_conn,
    light_connection const& connecting_light_conn) {
  return (duration)waiting_time_rules.waiting_time(
      waiting_time_rules.waiting_time_category(
          connecting_light_conn._full_con->con_info->family),
      waiting_time_rules.waiting_time_category(
          feeder_light_conn._full_con->con_info->family));
}

void pd_calc_data_departure::init_feeder_info(
    schedule const& schedule,
    std::vector<std::unique_ptr<train_distributions> > const&
        node_to_train_distributions) {
  auto const all_feeders_data =
      graph_accessor::get_all_potential_feeders(route_node_, light_connection_);

  for (unsigned int i = 0; i < all_feeders_data.size(); i++) {
    node const* feeder_route_node;
    light_connection const* feeder_light_conn;
    unsigned int feeder_distribution_pos;

    std::tie(feeder_route_node, feeder_light_conn, feeder_distribution_pos) =
        all_feeders_data[i];

    auto waiting_time = get_waiting_time(schedule.waiting_time_rules_,
                                         *feeder_light_conn, light_connection_);

    if (waiting_time > 0) {
      auto const transfer_time =
          schedule.stations[feeder_route_node->_station_node->_id]
              ->get_transfer_time();
      time const latest_feasible_arrival =
          (light_connection_.d_time + waiting_time) - transfer_time;

      probability_distribution const& feeder_distribution =
          node_to_train_distributions[feeder_route_node->_id]
              ->arrival_distributions_[feeder_distribution_pos];

      feeders_.emplace_back(feeder_distribution, feeder_light_conn->a_time,
                            latest_feasible_arrival, transfer_time);

      if (waiting_time > maximum_waiting_time_)
        maximum_waiting_time_ = waiting_time;
    }
  }
}

duration pd_calc_data_departure::get_largest_delay(void) const {
  duration maximum_train_delay = 0;
  if (is_first_route_node_) {
    maximum_train_delay =
        light_connection_.d_time +
        train_info_.first_departure_distribution->get_last_minute();
  } else {
    maximum_train_delay = (train_info_.preceding_arrival_info_.arrival_time_ +
                           train_info_.preceding_arrival_info_
                               .arrival_distribution_->get_last_minute() +
                           train_info_.preceding_arrival_info_.min_standing_) -
                          light_connection_.d_time;
  }

  return std::max(maximum_train_delay, maximum_waiting_time_);
}

void pd_calc_data_departure::debug_output(std::ostream& os) const {
  os << "scheduled-departure-time: " << light_connection_.d_time
     << " minimum-standing: "
     << train_info_.preceding_arrival_info_.min_standing_
     << " preceding-arrival-time: "
     << train_info_.preceding_arrival_info_.arrival_time_
     << " preceding-arrival-distribution: "
     << train_info_.preceding_arrival_info_.arrival_distribution_
     << " maximum-waiting-time: " << maximum_waiting_time_ << "\n";
  for (auto const& feeder : feeders_) {
    os << "Feeder -- arrival-time: " << feeder.arrival_time_
       << " distribution: " << &feeder.distribution_
       << " latest-feasible-arr: " << feeder.latest_feasible_arrival_
       << " transfer-time: " << feeder.transfer_time_ << "\n";
  }
  os << std::flush;
}

}  // namespace reliability
}  // namespace motis
