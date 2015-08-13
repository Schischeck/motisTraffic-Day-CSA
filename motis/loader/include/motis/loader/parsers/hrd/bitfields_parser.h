#pragma once

#include <map>
#include <bitset>

#include "flatbuffers/flatbuffers.h"

#include "parser/cstr.h"

#include "motis/loader/loaded_file.h"

constexpr int BIT_COUNT = 512;

namespace motis {
namespace loader {
namespace hrd {

constexpr int ALL_DAYS_KEY = 0;

std::bitset<BIT_COUNT> to_bitset(parser::cstr hex, char const* filename,
                                 int line_number);

std::map<int, flatbuffers::Offset<flatbuffers::String>> parse_bitfields(
    loaded_file, flatbuffers::FlatBufferBuilder& b);

}  // hrd
}  // loader
}  // motis
