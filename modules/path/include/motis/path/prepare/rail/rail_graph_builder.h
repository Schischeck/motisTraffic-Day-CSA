#pragma once

#include <string>

#include "motis/path/prepare/schedule/stations.h"

#include "motis/path/prepare/rail/rail_graph.h"
#include "motis/path/prepare/rail/rail_way.h"

namespace motis {
namespace path {

rail_graph build_rail_graph(std::vector<rail_way> const&,
                            std::vector<station> const&);

}  // namespace path
}  // namespace motis
