#pragma once

#include <bitset>
#include <map>

#include "parser/cstr.h"

#include "motis/loader/bitfield.h"
#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace hrd {

constexpr int ALL_DAYS_KEY = 0;

bitfield hex_str_to_bitset(parser::cstr hex, char const* filename,
                           int line_number);

std::map<int, bitfield> parse_bitfields(loaded_file const&);

}  // hrd
}  // loader
}  // motis
