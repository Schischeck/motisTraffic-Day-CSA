#pragma once

#include <string>

#include "parser/cstr.h"

#include "boost/filesystem/path.hpp"

#include "flatbuffers/flatbuffers.h"

namespace motis {
namespace loader {

void write_schedule(flatbuffers::FlatBufferBuilder& b,
                    boost::filesystem::path const& path);

inline flatbuffers::Offset<flatbuffers::String> to_fbs_string(
    flatbuffers::FlatBufferBuilder& b, parser::cstr s) {
  return b.CreateString(s.str, s.len);
}

}  // namespace loader
}  // namespace motis
