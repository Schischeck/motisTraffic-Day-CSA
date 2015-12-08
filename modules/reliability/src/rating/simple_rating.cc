#include "motis/reliability/rating/simple_rating.h"

#include "motis/core/journey/journey.h"
#include "motis/core/schedule/category.h"
#include "motis/core/schedule/schedule.h"

#include "motis/reliability/distributions/start_and_travel_distributions.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/rating/connection_rating.h"
#include "motis/reliability/rating/connection_to_graph_data.h"
#include "motis/reliability/realtime/time_util.h"

namespace motis {
namespace reliability {
namespace rating {
namespace simple_rating {

probability is_not_cancelled(unsigned int const /* family */) { return 0.995; }

probability_distribution get_travel_time_distribution(
    connection_element const& first_element_feeder,
    time const scheduled_arrival_time,
    start_and_travel_distributions const& s_t_distributions,
    schedule const& sched) {
  std::vector<start_and_travel_distributions::probability_distribution_cref>
      distributions;
  time const scheduled_departure_time = time_util::get_scheduled_event_time(
      *first_element_feeder.light_connection_,
      first_element_feeder.from_->get_station()->_id, time_util::departure,
      sched);
  int const departure_delay =
      std::max(0, first_element_feeder.light_connection_->d_time -
                      scheduled_departure_time);

  s_t_distributions.get_travel_time_distributions(
      sched.categories[first_element_feeder.light_connection_->_full_con
                           ->con_info->family]
          ->name,
      scheduled_arrival_time - scheduled_departure_time, departure_delay,
      distributions);
  if (distributions.empty()) {
    probability_distribution pd;
    pd.init_one_point(0, 1.0);
    return pd;
  }
  return distributions.back().get();
}

probability rate_interchange(
    connection_element const& first_element_feeder,
    connection_element const& last_element_feeder,
    connection_element const& first_element_departing_train,
    start_and_travel_distributions const& s_t_distributions,
    schedule const& schedule) {
  time const scheduled_departure_time = time_util::get_scheduled_event_time(
      *first_element_departing_train.light_connection_,
      first_element_departing_train.from_->get_station()->_id,
      time_util::departure, schedule);
  time const scheduled_arrival_time = time_util::get_scheduled_event_time(
      *last_element_feeder.light_connection_,
      last_element_feeder.to_->get_station()->_id, time_util::arrival,
      schedule);

  auto const& travel_time_distribution =
      get_travel_time_distribution(first_element_feeder, scheduled_arrival_time,
                                   s_t_distributions, schedule);
  time const latest_feasible_arrival =
      scheduled_departure_time -
      graph_accessor::get_interchange_time(*last_element_feeder.to_,
                                           *first_element_departing_train.from_,
                                           schedule);
  int const delay = (int)latest_feasible_arrival - (int)scheduled_arrival_time;
  return travel_time_distribution.probability_smaller_equal(delay);
}

void rate(simple_connection_rating& rating, journey const& journey,
          schedule const& schedule,
          start_and_travel_distributions const& s_t_distributions) {
  auto const connection_elements =
      rating::connection_to_graph_data::get_elements(schedule, journey);

  for (unsigned int idx = 0; idx < connection_elements.size(); ++idx) {
    auto const& ce = connection_elements[idx];
    rating.ratings_elements_.emplace_back(ce.front().departure_stop_idx_,
                                          ce.back().arrival_stop_idx());
    auto& element_ratings = rating.ratings_elements_.back().ratings_;
    element_ratings.emplace_back(
        rating_type::Cancellation,
        is_not_cancelled(
            ce.front().light_connection_->_full_con->con_info->family));
    if (idx > 0) {
      element_ratings.emplace_back(
          rating_type::Interchange,
          rate_interchange(connection_elements[idx - 1].front(),
                           connection_elements[idx - 1].back(), ce.front(),
                           s_t_distributions, schedule));
    }
  }

  rating.connection_rating_ = 1.0;
  for (auto const transport : rating.ratings_elements_) {
    for (auto const rating_prob : transport.ratings_) {
      rating.connection_rating_ *= rating_prob.second;
    }
  };
}

std::string to_string(rating_type const t) {
  switch (t) {
    case Cancellation: return "Cancellation";
    case Interchange: return "Interchange-Reliability";
  }
  return "unknown";
}

}  // namespace simple_rating
}  // namespace rating
}  // namespace reliability
}  // namespace motis
