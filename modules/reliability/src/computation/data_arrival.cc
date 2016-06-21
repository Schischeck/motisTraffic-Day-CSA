#include "motis/reliability/computation/data_arrival.h"

#include <climits>
#include <algorithm>

#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/access/realtime_access.h"

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
                                              event_type::DEP, schedule)) {
  init_arrival_time(arrival_node, light_connection, schedule);
  init_travel_info(light_connection, s_t_distributions, schedule.categories_);
}

void data_arrival::init_arrival_time(node const& route_node,
                                     light_connection const& light_conn,
                                     schedule const& sched) {
  auto const delay_info =
      get_delay_info(sched, &route_node, &light_conn, event_type::ARR);
  scheduled_arrival_time_ = delay_info.get_schedule_time();
  is_message_.received_ = (delay_info.get_reason() == timestamp_reason::IS);
  is_message_.current_time_ =
      is_message_.received_ ? delay_info.get_current_time() : 0;
}

void data_arrival::init_travel_info(
    light_connection const& light_connection,
    start_and_travel_distributions const& s_t_distributions,
    std::vector<std::unique_ptr<category>> const& categories) {
  assert(!departure_info_.distribution_.empty());

  s_t_distributions.get_travel_time_distributions(
      categories[light_connection.full_con_->con_info_->family_]->name_,
      scheduled_travel_duration(), departure_info_.distribution_.last_minute(),
      travel_distributions_);

  left_bound_ = INT_MAX;
  right_bound_ = INT_MIN;

  if (!travel_distributions_.empty()) {
    assert(departure_info_.distribution_.last_minute() >= 0);
    auto const last_minute =
        static_cast<unsigned>(departure_info_.distribution_.last_minute());
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
