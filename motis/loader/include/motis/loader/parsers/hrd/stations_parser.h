#pragma once

#include <map>

#include "motis/loader/loaded_file.h"
#include "motis/schedule-format/Station_generated.h"

namespace motis {
namespace loader {
namespace hrd {

std::map<int, flatbuffers::Offset<Station>> parse_stations(
    loaded_file station_names_file, loaded_file station_coordinates_file,
    loaded_file ds100_mappings_file, flatbuffers::FlatBufferBuilder& b);

}  // hrd
}  // loader
}  // motis
