#pragma once

#include <string>

#include "motis/path/prepare/rail/rail_graph.h"

namespace motis {
namespace path {

rail_graph load_rail_graph(std::string const& osm_file);

}  // namespace path
}  // namespace motis
