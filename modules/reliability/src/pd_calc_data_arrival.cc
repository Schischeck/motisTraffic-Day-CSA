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
    schedule const& schedule,  // XXX only category_names required?!
    train_distributions_container const& distributions_container,
    tt_distributions_manager const& tt_dist_manager)
    : route_node_(route_node), light_connection_(light_connection) {
  init_departure_info(distributions_container);
  init_travel_time_info(schedule.category_names, tt_dist_manager);
}

void pd_calc_data_arrival::init_departure_info(
    train_distributions_container const& distributions_container) {
  unsigned int route_node_id = 0;
  unsigned int distribution_pos = 0;
  std::tie(route_node_id, distribution_pos) =
      graph_accessor::get_departure_distribution_indices(route_node_,
                                                         light_connection_);
  departure_info_.distribution_ =
      &distributions_container.get_probability_distribution(
          route_node_id, distribution_pos,
          train_distributions_container::departure);
  departure_info_.scheduled_departure_time_ = light_connection_.d_time;
}

void pd_calc_data_arrival::init_travel_time_info(
    std::vector<std::string> const& category_names,
    tt_distributions_manager const& tt_dist_manager) {
  auto const& train_category =
      category_names[light_connection_._full_con->con_info->family];
  duration const travel_duration =
      std::min(get_scheduled_travel_duration(),
               (duration)(tt_dist_manager.longest_travel_time_ - 1));

  travel_time_info_ = &tt_dist_manager.get_travel_time_distributions(
      train_category, travel_duration);
}

time pd_calc_data_arrival::get_scheduled_arrival_time_() const {
  return light_connection_.a_time;
}

duration pd_calc_data_arrival::get_scheduled_travel_duration() const {
  return get_scheduled_arrival_time_() -
         departure_info_.scheduled_departure_time_;
}

void pd_calc_data_arrival::debug_output(std::ostream& os) const {
  os << get_scheduled_arrival_time_();
}

}  // namespace reliability
}  // namespace motis
