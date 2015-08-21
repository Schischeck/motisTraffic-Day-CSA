#pragma once

#include <map>
#include <bitset>

#include "parser/cstr.h"

#include "motis/loader/loaded_file.h"

constexpr int BIT_COUNT = 512;

namespace motis {
namespace loader {
namespace hrd {

constexpr int ALL_DAYS_KEY = 0;
typedef std::bitset<BIT_COUNT> bitfield;

bitfield hex_str_to_bitset(parser::cstr hex, char const* filename,
                           int line_number);

std::map<int, bitfield> parse_bitfields(loaded_file);

}  // hrd
}  // loader
}  // motis
