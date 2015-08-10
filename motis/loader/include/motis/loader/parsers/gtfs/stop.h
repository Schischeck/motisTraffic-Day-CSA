#pragma once

#include <vector>

#include "boost/filesystem/path.hpp"

#include "motis/schedule-format/Station_generated.h"

namespace motis {
namespace loader {
namespace gtfs {

std::vector<flatbuffers::Offset<Station>> read_stations(
    boost::filesystem::path const& path, flatbuffers::FlatBufferBuilder& b);

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
