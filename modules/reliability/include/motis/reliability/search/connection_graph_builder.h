#pragma once

namespace motis {
struct journey;
namespace reliability {
namespace search {
struct connection_graph;

namespace connection_graph_builder {
void add_journey(connection_graph&, journey const&);
}

}  // namespace search
}  // namespace reliability
}  // namespace motis
