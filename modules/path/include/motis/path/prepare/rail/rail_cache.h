#pragma once

#include "motis/path/prepare/rail/rail_way.h"

namespace motis {
namespace path {

std::vector<rail_way> load_rail_ways(std::string const& filename);

void store_rail_ways(std::string const& filename, std::vector<rail_way> const&);

}  // namespace path
}  // namespace motis
