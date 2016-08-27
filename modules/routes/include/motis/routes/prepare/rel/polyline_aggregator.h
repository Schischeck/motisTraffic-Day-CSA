#pragma once

#include "motis/core/common/geo.h"

#include "motis/routes/prepare/rel/relation_parser.h"

namespace motis {
namespace routes {

std::vector<std::vector<latlng>> aggregate_polylines(
    std::vector<relation> relations);

}  // namespace routes
}  // namespace motis
