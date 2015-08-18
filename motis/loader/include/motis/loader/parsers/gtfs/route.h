#pragma once

#include <tuple>
#include <map>

#include "flatbuffers/flatbuffers.h"

#include "parser/cstr.h"

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace gtfs {

std::map<int, flatbuffers::String> route_names(
    loaded_file, flatbuffers::FlatBufferBuilder& b);
}  // namespace gtfs
}  // namespace loader
}  // namespace motis
