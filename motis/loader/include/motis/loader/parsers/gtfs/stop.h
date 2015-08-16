#pragma once

#include <vector>

#include "motis/loader/loaded_file.h"
#include "motis/schedule-format/Station_generated.h"

namespace motis {
namespace loader {
namespace gtfs {

std::vector<flatbuffers::Offset<Station>> read_stations(
    loaded_file, flatbuffers::FlatBufferBuilder& b);

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
