#include "motis/reliability/pd_calc_data_arrival.h"

#include <algorithm>
#include <climits>

#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/schedule.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/train_distributions.h"

namespace motis {
namespace reliability {

pd_calc_data_arrival::pd_calc_data_arrival(
    node const& route_node, light_connection const& light_connection,
    schedule const& schedule,  // XXX only category_names required?!
    train_distributions_container const& distributions_container,
    start_and_travel_distributions const& s_t_distributions)
    : route_node_(route_node), light_connection_(light_connection) {
  init_departure_info(distributions_container);
  init_travel_info(s_t_distributions, schedule.category_names);
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

void pd_calc_data_arrival::init_travel_info(
    start_and_travel_distributions const& s_t_distributions,
    std::vector<std::string> const& category_names) {
  assert(!departure_info_.distribution_->empty());

  s_t_distributions.get_travel_time_distributions(
      category_names[light_connection_._full_con->con_info->family],
      scheduled_travel_duration(), departure_info_.distribution_->last_minute(),
      travel_distributions_);

  assert(travel_distributions_.size() ==
         departure_info_.distribution_->last_minute() + 1);

  left_bound_ = INT_MAX;
  right_bound_ = INT_MIN;
  for (unsigned int d = 0; d <= departure_info_.distribution_->last_minute();
       d++) {
    assert(!travel_distributions_[d].get().empty());
    int const left = departure_info_.distribution_->first_minute() +
                     travel_distributions_[d].get().first_minute();
    int const right = departure_info_.distribution_->last_minute() +
                      travel_distributions_[d].get().last_minute();
    if (left < left_bound_) {
      left_bound_ = left;
    }
    if (right > right_bound_) {
      right_bound_ = right;
    }
  }
}

time pd_calc_data_arrival::scheduled_arrival_time_() const {
  return light_connection_.a_time;
}

duration pd_calc_data_arrival::scheduled_travel_duration() const {
  return scheduled_arrival_time_() - departure_info_.scheduled_departure_time_;
}

void pd_calc_data_arrival::debug_output(std::ostream& os) const {
  os << scheduled_arrival_time_();
}

}  // namespace reliability
}  // namespace motis
