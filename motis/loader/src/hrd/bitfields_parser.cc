#include "motis/loader/parsers/hrd/bitfields_parser.h"

#include <string>
#include <algorithm>

#include "flatbuffers/util.h"

#include "parser/arg_parser.h"

#include "motis/core/common/logging.h"
#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"

using namespace parser;
using namespace motis::logging;

namespace motis {
namespace loader {
namespace hrd {

std::string hex_to_string(char c) {
  char str[2] = {c, '\0'};
  auto i = flatbuffers::StringToInt(str, 16);
  std::bitset<4> bits(i);
  return bits.to_string();
}

template <typename T>
std::string hex_to_string(T const& char_collection) {
  std::string bit_str;
  for (auto const& c : char_collection) {
    bit_str.append(hex_to_string(c));
  }
  return bit_str;
}

bitfield hex_str_to_bitset(cstr hex, char const* filename, int line_number) {
  std::string bit_str = hex_to_string(hex);
  auto period_begin = bit_str.find("11");
  auto period_end = bit_str.rfind("11");
  if (period_begin == std::string::npos || period_end == std::string::npos ||
      period_begin == period_end || period_end - period_begin <= 2) {
    throw parser_error(filename, line_number);
  }
  std::string bitstring(std::next(begin(bit_str), period_begin + 2),
                        period_end);
  std::reverse(begin(bitstring), end(bitstring));
  return bitfield{bitstring};
}

std::map<int, bitfield> parse_bitfields(loaded_file f) {
  scoped_timer timer("parsing bitfields");

  std::map<int, bitfield> bitfields;
  for_each_line_numbered(f.content, [&](cstr line, int line_number) {
    if (line.len == 0 || line.str[0] == '%') {
      return;
    } else if (line.len < 9) {
      throw parser_error(f.name, line_number);
    }

    bitfields[parse<int>(line.substr(0, size(6)))] =
        hex_str_to_bitset(line.substr(7), f.name, line_number);
  });

  bitfields[ALL_DAYS_KEY] = create_uniform_bitfield<BIT_COUNT>('1');

  return bitfields;
}

}  // hrd
}  // loader
}  // motis
