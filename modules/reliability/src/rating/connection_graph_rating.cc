#include "motis/reliability/rating/connection_graph_rating.h"

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/context.h"
#include "motis/reliability/distributions_container.h"
#include "motis/reliability/probability_distribution.h"
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

probability_distribution scheduled_transfer_filter(
    probability_distribution const& arrival_distribution,
    time const scheduled_arrival_time, time const scheduled_departure_time,
    duration const transfer_time, duration const waiting_time) {
  int const latest_arrival_delay =
      ((scheduled_departure_time + waiting_time) - transfer_time) -
      scheduled_arrival_time;
  std::vector<probability> values;
  if (latest_arrival_delay >= arrival_distribution.first_minute()) {
    for (int d = arrival_distribution.first_minute(); d <= latest_arrival_delay;
         ++d) {
      values.push_back(arrival_distribution.probability_equal(d));
    }
  } else {
    values.push_back(0.0);
  }
  probability_distribution pd;
  pd.init(values, arrival_distribution.first_minute());
  return pd;
}

probability_distribution compute_uncovered_arrival_distribution(
    probability_distribution const& arr_distribution,
    time const scheduled_arrival_time, time const scheduled_departure_time,
    duration const transfer_time, duration const waiting_time) {
  std::vector<probability> values;
  for (int d = arr_distribution.first_minute();
       d <= arr_distribution.last_minute(); ++d) {
    /* interchange is possible */
    if (scheduled_arrival_time + d + transfer_time <=
        scheduled_departure_time + waiting_time) {
      values.push_back(0.0);
    }
    /* interchange not possible */
    else {
      values.push_back(arr_distribution.probability_equal(d));
    }
  }
  probability_distribution pd;
  pd.init(values, arr_distribution.first_minute());
  return pd;
}

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
    context const& context) {
  auto connection_elements = rating::connection_to_graph_data::get_elements(
                                 context.schedule_, journey).second;
  connection_elements.insert(connection_elements.begin(), {arriving_element});

  std::vector<rating_element> ratings;
  ratings.emplace_back(arriving_element.departure_stop_idx_);
  ratings.back().arrival_distribution_ = arrival_distribution;

  public_transport::rate(ratings, connection_elements, true, context);

  return std::make_pair(ratings[1].departure_distribution_,
                        ratings.back().arrival_distribution_);
}

void rate_first_journey_in_cg(
    search::connection_graph const& cg,
    search::connection_graph::stop::alternative_info& alternative,
    context const& context) {
  connection_rating c_rating;
  rating::rate(c_rating, cg.journeys_.at(alternative.departing_journey_index_),
               context);
  alternative.rating_.departure_distribution_ =
      c_rating.public_transport_ratings_.front().departure_distribution_;
  alternative.rating_.arrival_distribution_ =
      c_rating.public_transport_ratings_.back().arrival_distribution_;
}
void rate_alternative_in_cg(
    search::connection_graph_search::detail::context::conn_graph_context&
        cg_context,
    search::connection_graph::stop const& stop,
    search::connection_graph::stop::alternative_info& alternative,
    context const& context) {
  auto& cg = *cg_context.cg_;
  auto const last_element = detail::find_arriving_connection_element(
      cg, stop.index_, context.schedule_);
  auto const& arrival_distribution =
      stop.alternative_infos_.size() <= 2
          ? last_element.second
          : cg_context.stop_states_.at(stop.index_)
                .uncovered_arrival_distribution_;
  std::tie(alternative.rating_.departure_distribution_,
           alternative.rating_.arrival_distribution_) =
      detail::rate(cg.journeys_.at(alternative.departing_journey_index_),
                   last_element.first, last_element.second, context);
}
}  // namespace detail

void rate_inserted_alternative(
    search::connection_graph_search::detail::context::conn_graph_context&
        cg_context,
    unsigned int const stop_idx, context const& context) {
  auto& cg = *cg_context.cg_;
  auto& stop = cg.stops_.at(stop_idx);
  auto& alternative = stop.alternative_infos_.back();

  /* first journey in the connection graph */
  if (stop_idx == search::connection_graph::stop::Index_departure_stop) {
    detail::rate_first_journey_in_cg(cg, alternative, context);
  }
  /* an alternative in the connection graph
   * (requires arrival distribution at stop) */
  else {
    detail::rate_alternative_in_cg(cg_context, stop, alternative, context);
  }

  if (alternative.head_stop_index_ !=
      search::connection_graph::stop::Index_arrival_stop) {
    return rate_inserted_alternative(cg_context, alternative.head_stop_index_,
                                     context);
  }
}

}  // namespace cg
}  // namespace rating
}  // namespace reliability
}  // namespace motis
