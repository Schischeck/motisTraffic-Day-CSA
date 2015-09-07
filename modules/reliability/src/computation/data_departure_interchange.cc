#include "motis/reliability/computation/data_departure_interchange.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/probability_distribution.h"
#include "motis/reliability/start_and_travel_distributions.h"

#include <cassert>

namespace motis {
namespace reliability {
namespace calc_departure_distribution {

/** get vector containing all potential feeders but not arriving_light_conn */
std::vector<std::unique_ptr<graph_accessor::feeder_info> >
get_all_potential_feeders_except_ic(
    node const& route_node, light_connection const& departing_light_conn,
    light_connection const& arriving_light_conn) {
  auto all_feeders = graph_accessor::get_all_potential_feeders(
      route_node, departing_light_conn);
  auto const it = std::find_if(
      all_feeders.begin(), all_feeders.end(),
      [arriving_light_conn](
          std::unique_ptr<graph_accessor::feeder_info> const& feeder) {
        return &feeder->light_conn_ == &arriving_light_conn;
      });
  if (it != all_feeders.end()) {
    all_feeders.erase(it);
  }
  return all_feeders;
}

data_departure_interchange::data_departure_interchange(
    bool const is_first_route_node, node const& route_node,
    light_connection const& departing_light_conn,
    light_connection const& arriving_light_conn,
    probability_distribution const& arrival_distribution,
    schedule const& schedule,
    distributions_container::precomputed_distributions_container const&
        precomputed_distributions,
    start_and_travel_distributions const& s_t_distributions)
    : data_departure(is_first_route_node, departing_light_conn.d_time) {
  init_train_info(route_node, departing_light_conn, schedule.category_names,
                  s_t_distributions, precomputed_distributions);
  init_interchange_feeder_info(arriving_light_conn, arrival_distribution);
  auto const all_feeders_data = get_all_potential_feeders_except_ic(
      route_node, departing_light_conn, arriving_light_conn);
  init_feeder_info(departing_light_conn, all_feeders_data, schedule,
                   precomputed_distributions);
}

void data_departure_interchange::init_interchange_feeder_info(
    light_connection const& arriving_light_conn,
    probability_distribution const& arrival_distribution) {
  interchange_feeder_info_.scheduled_arrival_time_ = arriving_light_conn.a_time;
  interchange_feeder_info_.arrival_distribution_ = &arrival_distribution;

  assert(interchange_feeder_info_.scheduled_arrival_time_ <=
         scheduled_departure_time_);
  assert(!interchange_feeder_info_.arrival_distribution_->empty());
}

}  // namespace calc_departure_distribution
}  // namespace reliability
}  // namespace motis
