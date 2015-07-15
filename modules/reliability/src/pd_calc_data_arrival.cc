#include "motis/reliability/pd_calc_data_arrival.h"

#include <algorithm>

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/tt_distributions_manager.h"

namespace motis {
namespace reliability {

pd_calc_data_arrival::pd_calc_data_arrival(
    tt_distributions_manager const& distributions_manager, node& route_node,
    light_connection const& light_connection, schedule const& schedule)
    : route_node_(route_node), light_connection_(light_connection) {

  auto const& train_category =
      schedule.category_names[light_connection_._full_con->con_info->family];

  duration const travel_duration =
      std::min(get_scheduled_travel_duration(),
               (duration)(distributions_manager.longest_travel_time_ - 1));

  std::tie(travel_time_info_.travel_time_distributions, /* XXX vector copy*/
           travel_time_info_.min_travel_delay_,
           travel_time_info_.max_travel_delay_) =
      distributions_manager.get_travel_time_distributions(train_category,
                                                          travel_duration);
}

probability_distribution const&
pd_calc_data_arrival::travel_time_info::get_travel_time_distribution(
    unsigned int departure_delay) const {
  return travel_time_distributions[std::min(
      departure_delay, max_departure_delay_ - 1)];
}

void pd_calc_data_arrival::debug_output(std::ostream& os) const {
  os << scheduled_arrival_time_;
}

}  // namespace reliability
}  // namespace motis
