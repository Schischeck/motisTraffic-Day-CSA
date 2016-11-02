#pragma once

#include <string>

#include "motis/core/common/typed_flatbuffer.h"
#include "motis/protocol/RoutesStationSeqRequest_generated.h"
#include "motis/routes/fbs/RouteIndex_generated.h"

namespace motis {
namespace routes {

struct lookup_index {
  using lookup_table = typed_flatbuffer<motis::routes::RouteLookup>;

  lookup_index(std::string const& s) : lookup_table_(s.size(), s.data()) {}

  std::string find(RoutesStationSeqRequest const* req);

  std::string find(std::vector<std::string> const& station_ids,
                   uint32_t const& clasz);

  lookup_table lookup_table_;
};

}  // namespace routes
}  // namespace motis
