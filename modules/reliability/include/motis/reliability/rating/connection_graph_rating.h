#pragma once

#include "motis/core/schedule/time.h"

#include "motis/reliability/search/cg_search_context.h"

namespace motis {
struct schedule;
namespace reliability {
struct context;
struct probability_distribution;
struct start_and_travel_distributions;
namespace distributions_container {
struct precomputed_distributions_container;
}
namespace search {
struct connection_graph;
}
namespace rating {
struct connection_element;
namespace cg {
void rate_inserted_alternative(
    search::connection_graph_search::detail::context::conn_graph_context&,
    unsigned int const stop_idx, context const&);

namespace detail {
struct interchange_info {
  interchange_info(connection_element const& arriving_element,
                   connection_element const& departing_element,
                   schedule const& sched);
  /* for tests */
  interchange_info(time const scheduled_arrival_time,
                   time const scheduled_departure_time,
                   duration const transfer_time, duration const waiting_time)
      : scheduled_arrival_time_(scheduled_arrival_time),
        scheduled_departure_time_(scheduled_departure_time),
        transfer_time_(transfer_time),
        waiting_time_(waiting_time) {}
  time scheduled_arrival_time_;
  time scheduled_departure_time_;
  duration transfer_time_;
  duration waiting_time_;
};
probability_distribution scheduled_transfer_filter(
    probability_distribution const& arrival_distribution,
    interchange_info const&);
/* This method computes which part of the arrival distribution
 * remains uncovered, in other words, leads to an invalid interchange.
 * @param probability_distribution const& arr_distribution used to
 * compute the departure distribution after the interchange.
 * @param interchange_info storing departure and arrival time
 * of the both involved trains. */
probability_distribution compute_uncovered_arrival_distribution(
    probability_distribution const& arr_distribution, interchange_info const&);
}
}  // namespace cg
}  // namespace rating
}  // namespace reliability
}  // namespace motis
