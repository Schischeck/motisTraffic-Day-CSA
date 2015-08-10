#pragma once

#include <vector>

#include "boost/filesystem/path.hpp"

#include "parser/cstr.h"

#include "motis/loader/loaded_file.h"
#include "motis/schedule-format/Station_generated.h"

namespace motis {
namespace loader {
namespace hrd {

std::vector<flatbuffers::Offset<Station>> parse_stations(
    loaded_file stations_file, loaded_file station_coordinates_file,
    flatbuffers::FlatBufferBuilder& b);

}  // hrd
}  // loader
}  // motis
