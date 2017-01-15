#pragma once

#include "motis/path/prepare/rail/rail_graph.h"

namespace motis {
namespace path {

std::vector<std::vector<rail_edge const*>> shortest_paths(
    rail_graph const&, std::vector<size_t> const& from,
    std::vector<size_t> const& to);

}  // namespace path
}  // namespace motis
