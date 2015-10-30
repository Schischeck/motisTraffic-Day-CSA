#include "motis/reliability/search/connection_graph.h"

namespace motis {
namespace reliability {
namespace search {
namespace connection_graph_search {
namespace simple_optimizer {
bool complete(connection_graph::stop const& stop,
              connection_graph const& conn_graph) {
  return std::count_if(conn_graph.journeys.begin(), conn_graph.journeys.end(),
                       [&](connection_graph::journey_info const& j) {
                         return j.from_index == stop.index;
                       }) == 3;
}

}  // namespace simple_optimizer
}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
