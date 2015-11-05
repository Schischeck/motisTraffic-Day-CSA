#pragma once

namespace motis {
struct schedule;
namespace reliability {
struct start_and_travel_distributions;
namespace distributions_container {
struct precomputed_distributions_container;
}
namespace search {
struct connection_graph;
}
namespace rating {
namespace cg {
void rate(search::connection_graph&, unsigned int const stop_idx,
          schedule const&,
          distributions_container::precomputed_distributions_container const&,
          start_and_travel_distributions const&);
}  // namespace cg
}  // namespace rating
}  // namespace reliability
}  // namespace motis
