#include "motis/reliability/rating/connection_graph_rating.h"

#include "motis/core/common/logging.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"
#include "motis/core/access/realtime_access.h"

#include "motis/reliability/context.h"
#include "motis/reliability/distributions/distributions_container.h"
#include "motis/reliability/distributions/probability_distribution.h"
#include "motis/reliability/distributions/start_and_travel_distributions.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/rating/connection_rating.h"
#include "motis/reliability/rating/connection_to_graph_data.h"
#include "motis/reliability/rating/public_transport.h"
#include "motis/reliability/realtime/time_util.h"
#include "motis/reliability/search/connection_graph.h"

namespace motis {
namespace reliability {
namespace rating {
namespace cg {
namespace detail {
interchange_info::interchange_info(connection_element const& arriving_element,
                                   connection_element const& departing_element,
                                   schedule const& sched) {
  if (arriving_element.light_connection_->a_time_ >
      departing_element.light_connection_->d_time_) {
    LOG(logging::error) << "unexpected arriving and departing element!";
  }
  arrival_time_ = arriving_element.light_connection_->a_time_;
  departure_time_ = departing_element.light_connection_->d_time_;

  auto const arr_delay_info =
      get_delay_info(sched, arriving_element.to_,
                     arriving_element.light_connection_, event_type::ARR);
  auto const dep_delay_info =
      get_delay_info(sched, departing_element.from_,
                     departing_element.light_connection_, event_type::DEP);

  scheduled_arrival_time_ = arr_delay_info.get_schedule_time();
  arrival_is_ = (arr_delay_info.get_reason() == timestamp_reason::IS);
  scheduled_departure_time_ = dep_delay_info.get_schedule_time();
  departure_is_ = (dep_delay_info.get_reason() == timestamp_reason::IS);

  transfer_time_ = graph_accessor::get_interchange_time(
      *arriving_element.to_, *departing_element.from_, sched);

  int const max_waiting = static_cast<int>(graph_accessor::get_waiting_time(
      sched.waiting_time_rules_, *arriving_element.light_connection_,
      *departing_element.light_connection_));
  int const departure_delay = static_cast<int>(departure_time_) -
                              static_cast<int>(scheduled_departure_time_);

  waiting_time_ =
      (!departure_is_ && departure_delay < static_cast<int>(max_waiting))
          ? max_waiting - departure_delay
          : 0;
}

probability_distribution scheduled_transfer_filter(
    probability_distribution const& arrival_distribution,
    interchange_info const& ic_info) {
  int const latest_arrival_delay =
      ((ic_info.departure_is_
            ? ic_info.departure_time_
            : ic_info.scheduled_departure_time_ + ic_info.waiting_time_) -
       ic_info.transfer_time_) -
      ic_info.scheduled_arrival_time_;
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

probability_distribution scheduled_transfer_filter(
    search::connection_graph_search::detail::context::conn_graph_context&
        cg_context,
    search::connection_graph::stop const& stop,
    probability_distribution const& train_arrival_distribution,
    interchange_info const& ic_info) {
  auto const& arrival_distribution =
      stop.alternative_infos_.size() == 1
          ? train_arrival_distribution
          : cg_context.stop_states_.at(stop.index_)
                .uncovered_arrival_distribution_;

  return scheduled_transfer_filter(arrival_distribution, ic_info);
}

probability_distribution compute_uncovered_arrival_distribution(
    probability_distribution const& arr_distribution,
    interchange_info const& ic_info) {
  bool is_empty_distribution = true;
  std::vector<probability> values;
  for (int d = arr_distribution.first_minute();
       d <= arr_distribution.last_minute(); ++d) {
    /* interchange was possible */
    if (ic_info.scheduled_arrival_time_ + d + ic_info.transfer_time_ <=
        (ic_info.departure_is_
             ? ic_info.departure_time_
             : ic_info.scheduled_departure_time_ + ic_info.waiting_time_)) {
      values.push_back(0.0);
    }
    /* interchange was not possible */
    else {
      values.push_back(arr_distribution.probability_equal(d));
      is_empty_distribution = false;
    }
  }
  probability_distribution pd;
  if (!is_empty_distribution) {
    pd.init(values, arr_distribution.first_minute());
  }
  return pd;
}

void update_uncovered_arrival_distribution(
    search::connection_graph_search::detail::context::conn_graph_context&
        cg_context,
    search::connection_graph::stop const& stop,
    probability_distribution const& distribution_arriving_alternative,
    interchange_info const& ic_info) {
  auto& uncovered_arr_dist =
      cg_context.stop_states_.at(stop.index_).uncovered_arrival_distribution_;
  if (stop.alternative_infos_.size() == 1) {
    uncovered_arr_dist = distribution_arriving_alternative;
  }
  uncovered_arr_dist =
      compute_uncovered_arrival_distribution(uncovered_arr_dist, ic_info);
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
          return alternative.next_stop_index_ == stop_idx;
        });
    if (it != stop.alternative_infos_.end()) {
      return std::make_pair(connection_to_graph_data::get_last_element(
                                schedule, cg.journeys_.at(it->journey_index_)),
                            it->rating_.arrival_distribution_);
    }
  }
  assert(false);
  return std::make_pair(connection_element(), probability_distribution());
}

std::pair<probability_distribution, probability_distribution> rate(
    std::vector<std::vector<connection_element>>& connection_elements,
    connection_element const& arriving_element,
    probability_distribution const& arrival_distribution,
    context const& context) {
  connection_elements.insert(connection_elements.begin(), {arriving_element});

  std::vector<rating_element> ratings;
  ratings.emplace_back(arriving_element.departure_stop_idx_);
  ratings.back().arrival_distribution_ = arrival_distribution;

  public_transport::rate(ratings, connection_elements, true, context);

  return std::make_pair(ratings[1].departure_distribution_,
                        ratings.back().arrival_distribution_);
}

void rate_first_journey_in_cg(
    search::connection_graph_search::detail::context::conn_graph_context&
        cg_context,
    search::connection_graph::stop::alternative_info& alternative,
    context const& context) {
  connection_rating c_rating;
  rating::rate(c_rating,
               cg_context.cg_->journeys_.at(alternative.journey_index_),
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
  auto const last_element = detail::find_arriving_connection_element(
      *cg_context.cg_, stop.index_, context.schedule_);
  auto const& alternative_journey =
      cg_context.cg_->journeys_.at(alternative.journey_index_);

  /* alternative to the destination consisting of a walk/mumo */
  if (std::find_if(alternative_journey.transports_.begin(),
                   alternative_journey.transports_.end(), [](auto const& t) {
                     return !t.is_walk_;
                   }) == alternative_journey.transports_.end()) {
    alternative.rating_.departure_distribution_ = last_element.second;
    alternative.rating_.arrival_distribution_ = last_element.second;
    cg_context.stop_states_.at(stop.index_).uncovered_arrival_distribution_ =
        probability_distribution();
    return;
  }

  auto connection_elements = rating::connection_to_graph_data::get_elements(
      context.schedule_, alternative_journey);

  interchange_info ic_info(last_element.first,
                           connection_elements.front().front(),
                           context.schedule_);
  auto const filtered_arrival_distribution =
      scheduled_transfer_filter(cg_context, stop, last_element.second, ic_info);

  /* rate departing alternative
   * note: this call modified the vector connection_elements */
  std::tie(alternative.rating_.departure_distribution_,
           alternative.rating_.arrival_distribution_) =
      rate(connection_elements, last_element.first,
           filtered_arrival_distribution, context);

  update_uncovered_arrival_distribution(cg_context, stop, last_element.second,
                                        ic_info);
}
}  // namespace detail

void rate_inserted_alternative(
    search::connection_graph_search::detail::context::conn_graph_context&
        cg_context,
    unsigned int const stop_idx, context const& context) {
  auto& stop = cg_context.cg_->stops_.at(stop_idx);
  auto& alternative = stop.alternative_infos_.back();

  /* first journey in the connection graph */
  if (stop_idx == search::connection_graph::stop::INDEX_DEPARTURE_STOP) {
    detail::rate_first_journey_in_cg(cg_context, alternative, context);
  }
  /* an alternative in the connection graph
   * (requires arrival distribution at stop) */
  else {
    detail::rate_alternative_in_cg(cg_context, stop, alternative, context);
  }

  if (alternative.next_stop_index_ !=
      search::connection_graph::stop::INDEX_ARRIVAL_STOP) {
    return rate_inserted_alternative(cg_context, alternative.next_stop_index_,
                                     context);
  }
}

}  // namespace cg
}  // namespace rating
}  // namespace reliability
}  // namespace motis
