#include "motis/reliability/pd_calc_data_departure.h"

#include <algorithm>

#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/waiting_time_rules.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/probability_distribution.h"
#include "motis/reliability/train_distributions.h"
#include "motis/reliability/tt_distributions_manager.h"

namespace motis {
namespace reliability {

pd_calc_data_departure::pd_calc_data_departure(
    node const& route_node, light_connection const& light_connection,
    bool const is_first_route_node, schedule const& schedule,
    train_distributions_container const& distributions_container,
    tt_distributions_manager const& tt_dist_manager)
    : route_node_(route_node),
      light_connection_(light_connection),
      is_first_route_node_(is_first_route_node),
      maximum_waiting_time_(0) {
  init_train_info(schedule.category_names, tt_dist_manager,
                  distributions_container);
  init_feeder_info(schedule, distributions_container);
}

void pd_calc_data_departure::init_train_info(
    std::vector<std::string> const& category_names,
    tt_distributions_manager const& tt_dist_manager,
    train_distributions_container const& distributions_container) {
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
        &distributions_container.get_train_distribution(
            route_node_._id, distribution_pos,
            train_distributions_container::arrival);

    // the standing-time is always less or equal 2 minutes
    train_info_.preceding_arrival_info_.min_standing_ =
        std::min(2, light_connection_.d_time - arriving_light_conn->a_time);
  }
}

void pd_calc_data_departure::init_feeder_info(
    schedule const& schedule,
    train_distributions_container const& distributions_container) {
  auto const all_feeders_data =
      graph_accessor::get_all_potential_feeders(route_node_, light_connection_);

  for (unsigned int i = 0; i < all_feeders_data.size(); i++) {
    node const* feeder_route_node;
    light_connection const* feeder_light_conn;
    unsigned int feeder_distribution_pos;

    std::tie(feeder_route_node, feeder_light_conn, feeder_distribution_pos) =
        all_feeders_data[i];

    auto waiting_time = graph_accessor::get_waiting_time(
        schedule.waiting_time_rules_, *feeder_light_conn, light_connection_);

    if (waiting_time > 0) {
      auto const transfer_time =
          schedule.stations[feeder_route_node->_station_node->_id]
              ->get_transfer_time();  // TODO: use track change time if possible
      time const latest_feasible_arrival =
          (light_connection_.d_time + waiting_time) - transfer_time;

      auto const& feeder_distribution =
          distributions_container.get_train_distribution(
              feeder_route_node->_id, feeder_distribution_pos,
              train_distributions_container::arrival);

      /** arrival distributions of the feeder trains
       * TODO: do not store the interchange feeder in this vector!
       * (distributions_calculator makes this assumption).
       */
      feeders_.emplace_back(feeder_distribution, feeder_light_conn->a_time,
                            latest_feasible_arrival, transfer_time);

      if (waiting_time > maximum_waiting_time_)
        maximum_waiting_time_ = waiting_time;
    }
  }
}

time pd_calc_data_departure::scheduled_departure_time() const {
  return light_connection_.d_time;
}

duration pd_calc_data_departure::largest_delay() const {
  duration maximum_train_delay = 0;
  if (is_first_route_node_) {
    maximum_train_delay =
        (duration)train_info_.first_departure_distribution->last_minute();
  } else {
    time const latest_arrival =
        train_info_.preceding_arrival_info_.arrival_time_ +
        train_info_.preceding_arrival_info_.arrival_distribution_
            ->last_minute() +
        train_info_.preceding_arrival_info_.min_standing_;
    maximum_train_delay = latest_arrival < light_connection_.d_time
                              ? 0
                              : latest_arrival - scheduled_departure_time();
  }
  return std::max(maximum_train_delay, maximum_waiting_time_);
}

void pd_calc_data_departure::debug_output(std::ostream& os) const {
  os << "pd_calc_data_departure:\n"
     << "route-node-id: " << route_node_._id
     << " station: " << route_node_._station_node->_id
     << "\nlight-connection: d" << format_time(light_connection_.d_time) << " a"
     << format_time(light_connection_.a_time) << " tr"
     << light_connection_._full_con->con_info->train_nr
     << "\nscheduled-departure-time: "
     << format_time(scheduled_departure_time())
     << " largest-delay: " << largest_delay()
     << " is-first-route-node: " << is_first_route_node_;

  if (is_first_route_node_) {
    os << "\nstart-distribution: " << *train_info_.first_departure_distribution;
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

}  // namespace reliability
}  // namespace motis
