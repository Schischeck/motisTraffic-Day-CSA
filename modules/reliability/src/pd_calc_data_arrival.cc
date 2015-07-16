#include "motis/reliability/pd_calc_data_arrival.h"

#include <algorithm>

#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/schedule.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/train_distributions.h"
#include "motis/reliability/tt_distributions_manager.h"

namespace motis {
namespace reliability {

pd_calc_data_arrival::pd_calc_data_arrival(
    node const& route_node, light_connection const& light_connection,
    schedule const& schedule, tt_distributions_manager const& tt_dist_manager,
    train_distributions_container const& distributions_container)
    : route_node_(route_node), light_connection_(light_connection) {

  /* departure info */

  unsigned int route_node_id = 0;
  unsigned int distribution_pos = 0;
  std::tie(route_node_id, distribution_pos) =
      graph_accessor::get_departure_distribution_indices(route_node_,
                                                         light_connection_);
  departure_info_.distribution_ =
      &distributions_container.get_train_distribution(
          route_node_id, distribution_pos,
          train_distributions_container::departure);
  departure_info_.scheduled_departure_time_ = light_connection_.d_time;

  /* travel time distribution */

  auto const& train_category =
      schedule.category_names[light_connection_._full_con->con_info->family];
  duration const travel_duration =
      std::min(get_scheduled_travel_duration(),
               (duration)(tt_dist_manager.longest_travel_time_ - 1));
  std::tie(travel_time_info_.travel_time_distributions, /* XXX vector copy*/
           travel_time_info_.max_departure_delay_,
           travel_time_info_.min_travel_delay_,
           travel_time_info_.max_travel_delay_) =
      tt_dist_manager.get_travel_time_distributions(train_category,
                                                    travel_duration);
}

time pd_calc_data_arrival::get_scheduled_arrival_time_() const {
  return light_connection_.a_time;
}

duration pd_calc_data_arrival::get_scheduled_travel_duration() const {
  return get_scheduled_arrival_time_() -
         departure_info_.scheduled_departure_time_;
}

probability_distribution const&
pd_calc_data_arrival::travel_time_info::get_travel_time_distribution(
    unsigned int departure_delay) const {
  return travel_time_distributions[std::min(departure_delay,
                                            max_departure_delay_ - 1)];
}

void pd_calc_data_arrival::debug_output(std::ostream& os) const {
  os << get_scheduled_arrival_time_();
}

}  // namespace reliability
}  // namespace motis
