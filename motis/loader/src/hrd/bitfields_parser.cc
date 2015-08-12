#include "motis/loader/parsers/hrd/bitfields_parser.h"

#include <string>
#include <algorithm>

#include "flatbuffers/util.h"

#include "parser/arg_parser.h"

#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"

using namespace parser;
using namespace flatbuffers;

namespace motis {
namespace loader {
namespace hrd {

std::string hex_to_string(char c) {
  char str[2] = {c, '\0'};
  auto i = StringToInt(str, 16);
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

std::bitset<BIT_COUNT> to_bitset(cstr hex, char const* filename,
                                 int line_number) {
  std::string bit_str = hex_to_string(hex);
  auto period_begin = bit_str.find("11");
  auto period_end = bit_str.rfind("11");
  if (period_begin == std::string::npos || period_end == std::string::npos ||
      period_begin == period_end || period_end - period_begin <= 2) {
    throw parser_error(filename, line_number);
  }
  return std::bitset<BIT_COUNT>{
      std::string(std::next(begin(bit_str), period_begin + 2),
                  std::next(begin(bit_str), period_end))};
}

std::map<int, Offset<String>> parse_bitfields(
    loaded_file f, flatbuffers::FlatBufferBuilder& b) {

  std::map<int, Offset<String>> bitfields;
  for_each_line_numbered(f.content, [&](cstr line, int line_number) {
    if (line.len == 0) {
      return;
    } else if (line.len < 9) {
      throw parser_error(f.name, line_number);
    }

    auto bitfield_num = parse<int>(line.substr(0, size(6)));
    auto bit_str = to_bitset(line.substr(7), f.name, line_number);
    bitfields[bitfield_num] =
        b.CreateString(bitset_to_string<BIT_COUNT>(bit_str));
  });

  std::string all_days_bit_str;
  all_days_bit_str.resize(BIT_COUNT);
  std::fill(begin(all_days_bit_str), end(all_days_bit_str), '1');
  std::bitset<BIT_COUNT> all_days(all_days_bit_str);
  bitfields[0] = b.CreateString(bitset_to_string<BIT_COUNT>(all_days));

  return bitfields;
}

}  // hrd
}  // loader
}  // motis
