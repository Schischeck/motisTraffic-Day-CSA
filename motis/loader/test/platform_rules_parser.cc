#include <cinttypes>
#include <cstring>
#include <bitset>

#include "catch/catch.hpp"

#include "parser/cstr.h"
#include "parser/arg_parser.h"

#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/parsers/hrd/files.h"
#include "motis/loader/parsers/hrd/platform_rules_parser.h"
#include "motis/loader/parsers/hrd/bitfields_parser.h"
#include "motis/schedule-format/Schedule_generated.h"

#include "./convert.h"

using namespace parser;

namespace motis {
namespace loader {
namespace hrd {

TEST_CASE("parse_platform_rules_1") {
  flatbuffers::FlatBufferBuilder b;

  auto bitfield_file_content = "000001 EF";
  auto bitfields = parse_bitfields({BITFIELDS_FILE, bitfield_file_content}, b);

  auto platform_file_content =
      "8509404 30467 85____ 3             000000\n"
      "8509404 30467 85____ 5             000001";
  auto plf_rules = parse_platform_rules({PLATFORMS_FILE, platform_file_content},
                                        bitfields, b);

  REQUIRE(plf_rules.size() == 1);

  auto key = std::make_tuple(8509404, 30467, string_to_int<uint64_t>("85____"));
  auto entry = plf_rules.find(key);

  REQUIRE(entry != plf_rules.end());

  auto rule_set = entry->second;

  REQUIRE(rule_set.size() == 2);

  // 8509404 30467 85____ 3             000000->[1111... == (0xFFF...)]
  REQUIRE(cstr(fbs_str_offset_to_str(rule_set[0].platform_name, b).c_str()) ==
          "3");

  std::string all_days_bit_str;
  all_days_bit_str.resize(BIT_COUNT);
  std::fill(begin(all_days_bit_str), end(all_days_bit_str), '1');
  std::bitset<BIT_COUNT> all_days(all_days_bit_str);

  REQUIRE(string_to_bitset<BIT_COUNT>(
              fbs_str_offset_to_str(rule_set[0].bitfield, b).c_str()) ==
          all_days);
  REQUIRE(rule_set[0].time == TIME_NOT_SET);

  // 8509404 30467 85____ 5             000001->[...01011 == (0xEF...)]
  REQUIRE(cstr(fbs_str_offset_to_str(rule_set[1].platform_name, b).c_str()) ==
          "5");
  REQUIRE(string_to_bitset<BIT_COUNT>(
              fbs_str_offset_to_str(rule_set[1].bitfield, b).c_str()) ==
          std::bitset<BIT_COUNT>("1011"));
  REQUIRE(rule_set[1].time == TIME_NOT_SET);
}

TEST_CASE("parse_platform_rules_2") {
  flatbuffers::FlatBufferBuilder b;

  auto bitfield_file_content = "000001 FF";
  auto bitfields = parse_bitfields({BITFIELDS_FILE, bitfield_file_content}, b);

  auto platform_file_content = "8000000 00001 80____ 1A       0130 000001";

  auto plf_rules = parse_platform_rules({PLATFORMS_FILE, platform_file_content},
                                        bitfields, b);

  REQUIRE(plf_rules.size() == 1);

  auto key = std::make_tuple(8000000, 1, string_to_int<uint64_t>("80____"));
  auto entry = plf_rules.find(key);

  REQUIRE(entry != plf_rules.end());

  auto rule_set = entry->second;

  REQUIRE(rule_set.size() == 1);

  // 800000 00001 80____ 1A       0130 000001->[...01111 == (0xFF)]
  REQUIRE(cstr(fbs_str_offset_to_str(rule_set[0].platform_name, b).c_str()) ==
          "1A");
  REQUIRE(string_to_bitset<BIT_COUNT>(
              fbs_str_offset_to_str(rule_set[0].bitfield, b).c_str()) ==
          std::bitset<BIT_COUNT>("1111"));
  REQUIRE(rule_set[0].time == 90);
}

}  // hrd
}  // loader
}  // motis
