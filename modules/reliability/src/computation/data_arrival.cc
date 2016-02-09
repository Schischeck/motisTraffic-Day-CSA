#include "motis/reliability/computation/data_arrival.h"

#include <algorithm>
#include <climits>

#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/schedule.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/realtime/time_util.h"

namespace motis {
namespace reliability {
namespace calc_arrival_distribution {

data_arrival::data_arrival(
    node const& departure_node, node const& arrival_node,
    light_connection const& light_connection,
    probability_distribution const& departure_distribution,
    schedule const& schedule,
    start_and_travel_distributions const& s_t_distributions)
    : departure_info_(
          departure_distribution,
          time_util::get_scheduled_event_time(departure_node, light_connection,
                                              time_util::departure, schedule)) {
  init_arrival_time(arrival_node, light_connection, schedule);
  init_travel_info(light_connection, s_t_distributions, schedule.categories);
}

void data_arrival::init_arrival_time(node const& route_node,
                                     light_connection const& light_conn,
                                     schedule const& sched) {
  is_message_.received_ = false;
  scheduled_arrival_time_ = light_conn.a_time;

  auto it = sched.graph_to_delay_info.find(graph_event(
      route_node.get_station()->_id, light_conn._full_con->con_info->train_nr,
      false, light_conn.a_time, route_node._route));
  if (it != sched.graph_to_delay_info.end()) {
    scheduled_arrival_time_ = it->second->_schedule_event._schedule_time;
    if (it->second->_reason == timestamp_reason::IS) {
      is_message_.received_ = true;
      is_message_.current_time_ = it->second->_current_time;
    }
  }
}

void data_arrival::init_travel_info(
    light_connection const& light_connection,
    start_and_travel_distributions const& s_t_distributions,
    std::vector<std::unique_ptr<category>> const& categories) {
  assert(!departure_info_.distribution_.empty());

  s_t_distributions.get_travel_time_distributions(
      categories[light_connection._full_con->con_info->family]->name,
      scheduled_travel_duration(), departure_info_.distribution_.last_minute(),
      travel_distributions_);

  left_bound_ = INT_MAX;
  right_bound_ = INT_MIN;

  if (travel_distributions_.size() > 0) {
    auto const last_minute =
        (unsigned int)departure_info_.distribution_.last_minute();
    assert(travel_distributions_.size() == last_minute + 1);
    for (unsigned int d = 0; d <= last_minute; d++) {
      assert(!travel_distributions_[d].get().empty());
      int const left = departure_info_.distribution_.first_minute() +
                       travel_distributions_[d].get().first_minute();
      int const right = departure_info_.distribution_.last_minute() +
                        travel_distributions_[d].get().last_minute();
      if (left < left_bound_) {
        left_bound_ = left;
      }
      if (right > right_bound_) {
        right_bound_ = right;
      }
    }
  }
}

duration data_arrival::scheduled_travel_duration() const {
  return scheduled_arrival_time_ - departure_info_.scheduled_departure_time_;
}

}  // namespace calc_arrival_distribution
}  // namespace reliability
}  // namespace motis
