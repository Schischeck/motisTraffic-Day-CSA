#pragma once

#include <vector>
#include <map>

#include "motis/loader/hrd/parser/station_meta_data_parser.h"

#include "motis/schedule-format/Footpath_generated.h"

namespace motis {
namespace loader {
namespace hrd {

flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Footpath>>>
create_footpaths(std::set<station_meta_data::footpath> const&,
                 std::map<int, flatbuffers::Offset<Station>> const&,
                 flatbuffers::FlatBufferBuilder&);

}  // hrd
}  // loader
}  // motis
