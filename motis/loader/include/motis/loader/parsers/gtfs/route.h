#pragma once

#include <tuple>

#include "parser/cstr.h"

namespace motis {
namespace loader {
namespace gtfs {

std::map<int, flatbuffers::String> route_names(
    loaded_file, flatbuffers::FlatBufferBuilder& b);
}  // namespace gtfs
}  // namespace loader
}  // namespace motis
