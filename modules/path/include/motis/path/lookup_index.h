#pragma once

#include <string>

#include "motis/core/common/typed_flatbuffer.h"

#include "motis/protocol/PathStationSeqRequest_generated.h"
#include "motis/path/fbs/PathIndex_generated.h"

namespace motis {
namespace path {

struct lookup_index {
  using lookup_table = typed_flatbuffer<motis::path::PathLookup>;

  lookup_index(std::string const& s) : lookup_table_(s.size(), s.data()) {}

  std::string find(PathStationSeqRequest const* req);

  std::string find(std::vector<std::string> const& station_ids,
                   uint32_t const& clasz);

  lookup_table lookup_table_;
};

}  // namespace path
}  // namespace motis
