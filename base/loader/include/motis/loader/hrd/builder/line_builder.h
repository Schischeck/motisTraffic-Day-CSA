#pragma once

#include <map>

#include "flatbuffers/flatbuffers.h"

#include "parser/cstr.h"

namespace motis {
namespace loader {
namespace hrd {

struct line_builder {
  flatbuffers::Offset<flatbuffers::String> get_or_create_line(
      std::vector<parser::cstr> const&, flatbuffers::FlatBufferBuilder&);

  std::map<uint64_t, flatbuffers::Offset<flatbuffers::String>> fbs_lines_;
};

}  // namespace hrd
}  // namespace loader
}  // namespace motis
