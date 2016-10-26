#pragma once

#include <string>

#include "motis/core/common/typed_flatbuffer.h"
#include "motis/protocol/RoutesStationSeqRequest_generated.h"
#include "motis/routes/fbs/RouteIndex_generated.h"

namespace motis {
namespace routes {

using lookup_table = typed_flatbuffer<motis::routes::RouteLookup>;

struct index_lookup {

  index_lookup(lookup_table const& lt);

  std::string lookup(RoutesStationSeqRequest const* req);

  lookup_table const& lookup_table_;
};

}  // namespace routes
}  // namespace motis
