#pragma once

#include <map>

#include "motis/loader/loaded_file.h"
#include "motis/schedule-format/Station_generated.h"
#include "motis/loader/parsers/hrd/station_meta_data_parser.h"

namespace motis {
namespace loader {
namespace hrd {

std::map<int, flatbuffers::Offset<Station>> parse_stations(
    loaded_file station_names_file, loaded_file station_coordinates_file,
    station_meta_data const&, flatbuffers::FlatBufferBuilder& b);

}  // hrd
}  // loader
}  // motis
