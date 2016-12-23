#pragma once

#include "motis/path/prepare/rail/rail_graph.h"

namespace motis {
namespace path {

rail_graph load_rail_graph(std::string const& filename);

void store_rail_graph(std::string const& filename, rail_graph const&);

}  // namespace path
}  // namespace motis
