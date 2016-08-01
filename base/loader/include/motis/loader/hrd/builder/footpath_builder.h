#pragma once

#include <map>
#include <vector>

#include "motis/loader/hrd/parser/station_meta_data_parser.h"

#include "motis/schedule-format/Footpath_generated.h"

namespace motis {
namespace loader {
namespace hrd {

flatbuffers64::Offset<flatbuffers64::Vector<flatbuffers64::Offset<Footpath>>>
create_footpaths(std::set<station_meta_data::footpath> const&,
                 std::map<int, flatbuffers64::Offset<Station>> const&,
                 flatbuffers64::FlatBufferBuilder&);

}  // namespace hrd
}  // namespace loader
}  // namespace motis
