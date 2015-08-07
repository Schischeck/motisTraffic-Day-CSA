#pragma once

#include <string>

#include "boost/filesystem/path.hpp"

#include "flatbuffers/flatbuffers.h"

namespace motis {
namespace loader {

void write_schedule(flatbuffers::FlatBufferBuilder& b,
                    boost::filesystem::path const& path);

}  // namespace loader
}  // namespace motis
