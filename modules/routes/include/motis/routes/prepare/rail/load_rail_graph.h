#pragma once

#include <string>

#include "motis/routes/prepare/rail/rail_graph.h"

namespace motis {
namespace routes {

rail_graph load_rail_graph(std::string const& osm_file);

}  // namespace routes
}  // namespace motis
