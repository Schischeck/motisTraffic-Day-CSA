#pragma once

#include <map>
#include <vector>

#include "motis/loader/hrd/parser/station_meta_data_parser.h"

#include "motis/schedule-format/MetaStation_generated.h"

namespace motis {
namespace loader {
namespace hrd {

flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<MetaStation>>>
create_meta_stations(std::set<station_meta_data::meta_station> const&,
                     std::map<int, flatbuffers::Offset<Station>> const&,
                     flatbuffers::FlatBufferBuilder&);

}  // hrd
}  // loader
}  // motis
