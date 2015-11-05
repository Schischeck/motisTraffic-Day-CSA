#include "motis/reliability/rating/connection_graph_rating.h"

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/distributions_container.h"
#include "motis/reliability/start_and_travel_distributions.h"
#include "motis/reliability/rating/connection_rating.h"
#include "motis/reliability/rating/connection_to_graph_data.h"
#include "motis/reliability/rating/public_transport.h"
#include "motis/reliability/search/connection_graph.h"

namespace motis {
namespace reliability {
namespace rating {
namespace cg {
namespace detail {

std::pair<connection_element, probability_distribution>
find_arriving_connection_element(search::connection_graph const& cg,
                                 unsigned int const stop_idx,
                                 schedule const& schedule) {
  for (auto const& stop : cg.stops_) {
    auto it = std::find_if(
        stop.alternative_infos_.begin(), stop.alternative_infos_.end(),
        [stop_idx](search::connection_graph::stop::alternative_info const&
                       alternative) {
          return alternative.head_stop_index_ == stop_idx;
        });
    if (it != stop.alternative_infos_.end()) {
      return std::make_pair(
          connection_to_graph_data::get_last_element(
              schedule, cg.journeys_.at(it->departing_journey_index_)),
          it->rating_.arrival_distribution_);
    }
  }
  assert(false);
  return std::make_pair(connection_element(), probability_distribution());
}

std::pair<probability_distribution, probability_distribution> rate(
    journey const& journey, connection_element const& arriving_element,
    probability_distribution const& arrival_distribution,
    schedule const& schedule,
    distributions_container::precomputed_distributions_container const&
        precomputed_distributions,
    start_and_travel_distributions const& s_t_distributions) {
  auto connection_elements =
      rating::connection_to_graph_data::get_elements(schedule, journey).second;
  connection_elements.insert(connection_elements.begin(), {arriving_element});

  std::vector<rating_element> ratings;
  ratings.emplace_back(arriving_element.departure_stop_idx_);
  ratings.back().arrival_distribution_ = arrival_distribution;

  public_transport::rate(ratings, connection_elements, schedule,
                         precomputed_distributions, s_t_distributions);

  return std::make_pair(ratings[1].departure_distribution_,
                        ratings.back().arrival_distribution_);
}
}  // namespace detail

void rate(search::connection_graph& cg, unsigned int const stop_idx,
          schedule const& schedule,
          distributions_container::precomputed_distributions_container const&
              precomputed_distributions,
          start_and_travel_distributions const& s_t_distributions) {
  auto& alternative = cg.stops_.at(stop_idx).alternative_infos_.back();

  /* first journey in the connection graph */
  if (stop_idx == search::connection_graph::stop::Index_departure_stop) {
    connection_rating c_rating;
    rating::rate(c_rating,
                 cg.journeys_.at(alternative.departing_journey_index_),
                 schedule, precomputed_distributions, s_t_distributions);
    alternative.rating_.departure_distribution_ =
        c_rating.public_transport_ratings_.front().departure_distribution_;
    alternative.rating_.arrival_distribution_ =
        c_rating.public_transport_ratings_.back().arrival_distribution_;

  }
  /* an alternative in the connection graph
   * (requires arrival distribution at stop) */
  else {
    auto const last_element =
        detail::find_arriving_connection_element(cg, stop_idx, schedule);
    std::tie(alternative.rating_.departure_distribution_,
             alternative.rating_.arrival_distribution_) =
        detail::rate(cg.journeys_.at(alternative.departing_journey_index_),
                     last_element.first, last_element.second, schedule,
                     precomputed_distributions, s_t_distributions);
  }

  if (alternative.head_stop_index_ !=
      search::connection_graph::stop::Index_arrival_stop) {
    return rate(cg, alternative.head_stop_index_, schedule,
                precomputed_distributions, s_t_distributions);
  }
}

}  // namespace cg
}  // namespace rating
}  // namespace reliability
}  // namespace motis
