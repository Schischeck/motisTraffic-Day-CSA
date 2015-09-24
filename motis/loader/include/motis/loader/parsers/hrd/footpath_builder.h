#pragma once

#include <vector>

#include "motis/schedule-format/Footpath_generated.h"
#include "motis/loader/parsers/hrd/station_meta_data_parser.h"

namespace motis {
namespace loader {
namespace hrd {

flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Footpath>>>
build_footpaths(std::vector<station_meta_data::footpath> const&,
                std::map<int, flatbuffers::Offset<Station>> const&,
                flatbuffers::FlatBufferBuilder& b);

}  // hrd
}  // loader
}  // motis
