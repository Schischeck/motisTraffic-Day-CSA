#include "motis/reliability/search/connection_graph.h"

namespace motis {
namespace reliability {
namespace search {
namespace connection_graph_search {
namespace simple_optimizer {
bool complete(connection_graph::stop const& stop) {
  return stop.departure_infos.size() >= 3;
}

}  // namespace simple_optimizer
}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
