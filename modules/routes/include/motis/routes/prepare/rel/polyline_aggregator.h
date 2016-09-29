#pragma once

#include "motis/geo/polyline.h"

#include "motis/routes/prepare/rel/relation_parser.h"

namespace motis {
namespace routes {

std::vector<geo::polyline> aggregate_polylines(std::vector<relation> const&);

}  // namespace routes
}  // namespace motis
