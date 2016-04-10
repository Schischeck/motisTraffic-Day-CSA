#pragma once

#include <map>
#include <vector>

#include "motis/loader/hrd/parser/station_meta_data_parser.h"

#include "motis/schedule-format/Footpath_generated.h"

namespace motis {
namespace loader {
namespace hrd {

flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Footpath>>>
create_footpaths(std::set<station_meta_data::footpath> const&,
                 std::map<int, flatbuffers::Offset<Station>> const&,
                 flatbuffers::FlatBufferBuilder&);

}  // namespace hrd
}  // namespace loader
}  // namespace motis
