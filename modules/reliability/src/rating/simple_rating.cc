#include "motis/reliability/rating/simple_rating.h"

#include "motis/core/common/journey.h"
#include "motis/core/schedule/category.h"
#include "motis/core/schedule/schedule.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/rating/connection_rating.h"
#include "motis/reliability/rating/connection_to_graph_data.h"
#include "motis/reliability/start_and_travel_distributions.h"

namespace motis {
namespace reliability {
namespace rating {
namespace simple_rating {

probability is_not_cancelled(unsigned int const /* family */) { return 0.995; }

probability_distribution const& get_travel_time_distribution(
    connection_element const& first_element_feeder,
    connection_element const& last_element_feeder,
    start_and_travel_distributions const& s_t_distributions,
    std::vector<std::unique_ptr<category>> const& categories) {
  std::vector<start_and_travel_distributions::probability_distribution_cref>
      distributions;
  s_t_distributions.get_travel_time_distributions(
      categories[first_element_feeder.light_connection_->_full_con->con_info
                     ->family]->name,
      last_element_feeder.light_connection_->a_time -
          first_element_feeder.light_connection_->d_time,
      0 /* todo: for realtime, use the departure delay */, distributions);
  return distributions.back().get();
}

probability rate_interchange(
    connection_element const& first_element_feeder,
    connection_element const& last_element_feeder,
    connection_element const& first_element_departing_train,
    start_and_travel_distributions const& s_t_distributions,
    schedule const& schedule) {
  auto const& travel_time_distribution =
      get_travel_time_distribution(first_element_feeder, last_element_feeder,
                                   s_t_distributions, schedule.categories);
  time const latest_feasible_arrival =
      first_element_departing_train.light_connection_->d_time -
      graph_accessor::get_interchange_time(*last_element_feeder.to_,
                                           *first_element_departing_train.from_,
                                           schedule);
  int const delay =
      (int)latest_feasible_arrival -
      (int)last_element_feeder.light_connection_
          ->a_time; /* todo: for realtime, use the scheduled arrival time */

  return travel_time_distribution.probability_smaller_equal(delay);
}

bool rate(simple_connection_rating& rating, journey const& journey,
          schedule const& schedule,
          start_and_travel_distributions const& s_t_distributions) {
  auto const connection_elements =
      rating::connection_to_graph_data::get_elements(schedule, journey);
  if (!connection_elements.first) {
    return false;
  }

  for (unsigned int idx = 0; idx < connection_elements.second.size(); ++idx) {
    auto const& ce = connection_elements.second[idx];
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
          rate_interchange(connection_elements.second[idx - 1].front(),
                           connection_elements.second[idx - 1].back(),
                           ce.front(), s_t_distributions, schedule));
    }
  }

  rating.connection_rating_ = 1.0;
  for (auto const transport : rating.ratings_elements_) {
    for (auto const rating_prob : transport.ratings_) {
      rating.connection_rating_ *= rating_prob.second;
    }
  }
  return true;
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
