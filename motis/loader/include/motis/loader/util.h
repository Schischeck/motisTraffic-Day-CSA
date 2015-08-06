#pragma once

#include <string>

#include "flatbuffers/flatbuffers.h"

namespace motis {
namespace loader {

void write_schedule(flatbuffers::FlatBufferBuilder& b, std::string const& path);

}  // namespace loader
}  // namespace motis
