#pragma once

#include <string>

#include "motis/core/common/typed_flatbuffer.h"
#include "motis/protocol/RoutesStationSeqRequest_generated.h"
#include "motis/routes/fbs/RouteIndex_generated.h"

namespace motis {
namespace routes {

using lookup_table = typed_flatbuffer<motis::routes::RouteLookup>;

std::string lookup_index(lookup_table const& lookup,
                         RoutesStationSeqRequest const* req);

std::string lookup_index(lookup_table const& lookup,
                         std::vector<std::string> const& station_ids,
                         uint32_t const& clasz);

}  // namespace routes
}  // namespace motis
