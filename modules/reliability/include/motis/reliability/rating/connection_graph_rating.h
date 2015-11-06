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
namespace rating {
namespace cg {
void rate_inserted_alternative(
    search::connection_graph_search::detail::context::conn_graph_context&,
    unsigned int const stop_idx, context const&);

namespace detail {
probability_distribution scheduled_transfer_filter(
    probability_distribution const& arrival_distribution,
    time const scheduled_arrival_time, time const scheduled_departure_time,
    duration const transfer_time, duration const waiting_time);
probability_distribution compute_uncovered_arrival_distribution(
    probability_distribution const& arr_distribution,
    time const scheduled_arrival_time, time const scheduled_departure_time,
    duration const transfer_time, duration const waiting_time);
}
}  // namespace cg
}  // namespace rating
}  // namespace reliability
}  // namespace motis
