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

data_departure::data_departure(
    node const& route_node, light_connection const& light_connection,
    bool const is_first_route_node,
    distributions_container::container const& distributions_preceding_arrival,
    distributions_container::container::node const&
        departing_distributions_node,
    reliability::context const& context)
    : is_first_route_node_(is_first_route_node), maximum_waiting_time_(0) {
  init_departure_time(route_node, light_connection, context.schedule_);
  if (is_first_route_node) {
    init_first_departure_info(light_connection, context.s_t_distributions_,
                              context.schedule_.categories_);
  } else {
    init_preceding_arrival_info(route_node, light_connection.d_time_,
                                distributions_preceding_arrival,
                                context.schedule_);
  }
  init_feeder_info(route_node, light_connection,
                   departing_distributions_node.predecessors_,
                   context.schedule_);
}

data_departure::data_departure(node const& route_node,
                               light_connection const& light_connection,
                               bool const is_first_route_node,
                               schedule const& sched)
    : is_first_route_node_(is_first_route_node), maximum_waiting_time_(0) {
  init_departure_time(route_node, light_connection, sched);
}

void data_departure::init_departure_time(node const& route_node,
                                         light_connection const& light_conn,
                                         schedule const& sched) {
  auto const delay_info = get_delay_info(
      sched, graph_accessor::get_departing_route_edge(route_node), &light_conn,
      event_type::DEP);
  scheduled_departure_time_ = delay_info.get_schedule_time();
  is_message_.received_ = (delay_info.get_reason() == delay_info::reason::IS);
  is_message_.current_time_ =
      is_message_.received_ ? delay_info.get_current_time() : 0;
}

void data_departure::init_first_departure_info(
    light_connection const& departing_light_conn,
    start_and_travel_distributions const& s_t_distributions,
    std::vector<std::unique_ptr<category>> const& categories) {
  auto const& train_category =
      categories[departing_light_conn.full_con_->con_info_->family_]->name_;
  train_info_.first_departure_distribution_ =
      &s_t_distributions.get_start_distribution(train_category).second.get();
  assert(!train_info_.first_departure_distribution_->empty());
}

void data_departure::init_preceding_arrival_info(
    node const& route_node, motis::time const departure_time,
    distributions_container::container const& distributions_container,
    schedule const& sched) {
  auto const& arriving_light_conn =
      graph_accessor::get_previous_light_connection(
          graph_accessor::get_arriving_route_edge(route_node)
              ->m_.route_edge_.conns_,
          departure_time);

  train_info_.preceding_arrival_info_.scheduled_arrival_time_ =
      time_util::get_scheduled_event_time(route_node, arriving_light_conn,
                                          time_util::arrival, sched);
  train_info_.preceding_arrival_info_.arrival_distribution_ =
      &distributions_container.get_distribution(
          distributions_container::to_container_key(
              arriving_light_conn, route_node.get_station()->id_,
              time_util::arrival,
              train_info_.preceding_arrival_info_.scheduled_arrival_time_,
              sched));
  // the standing-time is always less or equal 2 minutes
  train_info_.preceding_arrival_info_.min_standing_ = std::min(
      2, scheduled_departure_time_ -
             train_info_.preceding_arrival_info_.scheduled_arrival_time_);

  assert(train_info_.preceding_arrival_info_.scheduled_arrival_time_ <=
         scheduled_departure_time_);
  assert(!train_info_.preceding_arrival_info_.arrival_distribution_->empty());
}

void erase_preceding_arrival(
    node const& route_node, light_connection const& departing_light_conn,
    std::vector<distributions_container::container::node*>& predecessors,
    motis::time const preceding_arrival_time_scheduled,
    schedule const& schedule) {
  auto const& arriving_light_conn =
      graph_accessor::get_previous_light_connection(
          graph_accessor::get_arriving_route_edge(route_node)
              ->m_.route_edge_.conns_,
          departing_light_conn.d_time_);
  auto const preceding_arrival_key = distributions_container::to_container_key(
      arriving_light_conn, route_node.get_station()->id_, time_util::arrival,
      preceding_arrival_time_scheduled, schedule);

  predecessors.erase(
      std::remove_if(
          predecessors.begin(), predecessors.end(),
          [&](distributions_container::container::node const* feeder) {
            return feeder->key_ == preceding_arrival_key;
          }),
      predecessors.end());
}

void data_departure::init_feeder_info(
    node const& route_node, light_connection const& light_conn,
    std::vector<distributions_container::container::node*> const& predecessors,
    schedule const& schedule) {
  std::vector<distributions_container::container::node*> feeders(predecessors);
  if (!is_first_route_node_) {
    erase_preceding_arrival(
        route_node, light_conn, feeders,
        train_info_.preceding_arrival_info_.scheduled_arrival_time_, schedule);
  }

  for (auto const& feeder : feeders) {
    /* TODO: test whether feeder is cancelled or rerouted!
     * (do the same for preceding arrival or interchange feeder?!)
     */
    auto const waiting_time = graph_accessor::get_waiting_time(
        schedule.waiting_time_rules_,
        graph_accessor::find_family(schedule.categories_,
                                    feeder->key_.category_)
            .second,
        light_conn.full_con_->con_info_->family_);
    auto const transfer_time =
        schedule.stations_[feeder->key_.station_index_]->transfer_time_;
    time const latest_feasible_arrival =
        (scheduled_departure_time_ + waiting_time) - transfer_time;

    feeders_.emplace_back(feeder->pd_, feeder->key_.scheduled_event_time_,
                          latest_feasible_arrival, transfer_time);

    if (maximum_waiting_time_ < waiting_time) {
      maximum_waiting_time_ = waiting_time;
    }
  }  // end of for all_feeders_data
}

duration data_departure::largest_delay() const {
  duration maximum_train_delay = 0;
  if (is_first_route_node_) {
    maximum_train_delay = static_cast<duration>(
        train_info_.first_departure_distribution_->last_minute());
  } else {
    time const latest_arrival =
        train_info_.preceding_arrival_info_.scheduled_arrival_time_ +
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
       << format_time(
              train_info_.preceding_arrival_info_.scheduled_arrival_time_)
       << " min-standing: " << train_info_.preceding_arrival_info_.min_standing_
       << "\npreceding-arrival-distribution: "
       << *train_info_.preceding_arrival_info_.arrival_distribution_;
  }

  os << "\nFeeders:";

  for (auto const& feeder : feeders_) {
    os << "\nfeeder-arrival-time: "
       << format_time(feeder.scheduled_arrival_time_)
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
