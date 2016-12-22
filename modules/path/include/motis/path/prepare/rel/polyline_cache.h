#pragma once

#include "motis/path/prepare/rel/polyline_aggregator.h"

namespace motis {
namespace path {

std::vector<aggregated_polyline> load_relation_polylines(
    std::string const& filename);

void store_relation_polylines(std::string const& filename,
                              std::vector<aggregated_polyline> const&);

}  // namespace path
}  // namespace motis
