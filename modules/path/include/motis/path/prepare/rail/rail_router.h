#pragma once

#include "motis/path/prepare/rail/rail_graph.h"
#include "motis/path/prepare/rail/rail_path.h"
#include "motis/path/prepare/rail/rail_phantom.h"

namespace motis {
namespace path {

std::vector<rail_path> shortest_paths(rail_graph const& graph,
                                      rail_phantom const& from,
                                      std::vector<rail_phantom> const& to);

}  // namespace path
}  // namespace motis
