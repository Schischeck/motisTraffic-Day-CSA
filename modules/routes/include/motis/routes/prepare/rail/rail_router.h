#pragma once

#include "motis/routes/prepare/rail/rail_graph.h"

namespace motis {
namespace routes {

std::vector<std::vector<rail_link const*>> shortest_paths(
    rail_graph const& graph, std::vector<size_t> const& from,
    std::vector<size_t> const& to);

}  // namespace routes
}  // namespace motis
