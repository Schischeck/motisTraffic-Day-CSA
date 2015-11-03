#include "motis/reliability/search/connection_graph.h"

namespace motis {
namespace reliability {
namespace search {
namespace connection_graph_search {
namespace simple_optimizer {
inline bool complete(connection_graph::stop const& stop,
                     connection_graph const&) {
  return stop.departure_infos_.size() >= 3;
}

}  // namespace simple_optimizer
}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
