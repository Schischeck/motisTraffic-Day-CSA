#pragma once

#include <vector>

namespace motis {
struct journey;
namespace reliability {
namespace search {
struct connection_graph;

namespace connection_graph_builder {
void add_base_journey(connection_graph&, journey const&);
void add_alternative_journey(connection_graph&, unsigned int const stop_idx,
                             journey const&);

namespace detail {
/* split journey at each stop with interchange */
std::vector<journey> split_journey(journey const&);
journey remove_dummy_stops(journey const&);
journey move_early_walk(journey const&);
}
}

}  // namespace search
}  // namespace reliability
}  // namespace motis
