#include <cinttypes>
#include <cstring>
#include <bitset>

#include "gtest/gtest.h"

#include "parser/cstr.h"
#include "parser/arg_parser.h"

#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/parsers/hrd/files.h"
#include "motis/loader/parsers/hrd/platform_rules_parser.h"
#include "motis/loader/parsers/hrd/bitfields_parser.h"
#include "motis/schedule-format/Schedule_generated.h"

#include "../convert.h"

using namespace parser;

namespace motis {
namespace loader {
namespace hrd {

TEST(loader_hrd_platform_rules, parse_platform_rules_1) {
  flatbuffers::FlatBufferBuilder b;

  auto bitfield_file_content = "000001 EF";
  auto bitfields = parse_bitfields({BITFIELDS_FILE, bitfield_file_content});

  auto platform_file_content =
      "8509404 30467 85____ 3             000000\n"
      "8509404 30467 85____ 5             000001";
  auto plf_rules =
      parse_platform_rules({PLATFORMS_FILE, platform_file_content}, b);

  ASSERT_TRUE(plf_rules.size() == 1);

  auto key = std::make_tuple(8509404, 30467, raw_to_int<uint64_t>("85____"));
  auto entry = plf_rules.find(key);

  ASSERT_TRUE(entry != end(plf_rules));

  auto rule_set = entry->second;

  ASSERT_TRUE(rule_set.size() == 2);
  ASSERT_TRUE(cstr(to_string(rule_set[0].platform_name, b).c_str()) == "3");

  std::string all_days_bit_str;
  all_days_bit_str.resize(BIT_COUNT);
  std::fill(begin(all_days_bit_str), end(all_days_bit_str), '1');
  std::bitset<BIT_COUNT> all_days(all_days_bit_str);

  ASSERT_TRUE(rule_set[0].bitfield_num == 0);
  ASSERT_TRUE(rule_set[0].time == TIME_NOT_SET);

  ASSERT_TRUE(cstr(to_string(rule_set[1].platform_name, b).c_str()) == "5");
  ASSERT_TRUE(rule_set[1].bitfield_num == 1);
  ASSERT_TRUE(rule_set[1].time == TIME_NOT_SET);
}

TEST(loader_hrd_platform_rules, parse_platform_rules_2) {
  flatbuffers::FlatBufferBuilder b;

  auto bitfield_file_content = "000001 FF";
  auto bitfields = parse_bitfields({BITFIELDS_FILE, bitfield_file_content});

  auto platform_file_content = "8000000 00001 80____ 1A       0130 000001";

  auto plf_rules =
      parse_platform_rules({PLATFORMS_FILE, platform_file_content}, b);

  ASSERT_TRUE(plf_rules.size() == 1);

  auto key = std::make_tuple(8000000, 1, raw_to_int<uint64_t>("80____"));
  auto entry = plf_rules.find(key);

  ASSERT_TRUE(entry != plf_rules.end());

  auto rule_set = entry->second;

  ASSERT_TRUE(rule_set.size() == 1);

  // 800000 00001 80____ 1A       0130 000001->[...01111 == (0xFF)]
  ASSERT_TRUE(cstr(to_string(rule_set[0].platform_name, b).c_str()) == "1A");
  ASSERT_TRUE(rule_set[0].bitfield_num == 1);
  ASSERT_TRUE(rule_set[0].time == 90);
}

TEST(loader_hrd_platform_rules, parse_platform_rules_line_too_short) {
  bool catched = false;
  try {
    flatbuffers::FlatBufferBuilder b;

    auto bitfield_file_content = "000001 EF";
    auto bitfields = parse_bitfields({BITFIELDS_FILE, bitfield_file_content});

    auto platform_file_content =
        "8509404 30467 85____ 3             000000\n"
        "8509404 30467 85____ 5             00000";
    auto plf_rules =
        parse_platform_rules({PLATFORMS_FILE, platform_file_content}, b);
  } catch (parser_error const& e) {
    ASSERT_TRUE(e.line_number == 2);
    ASSERT_TRUE(e.filename == PLATFORMS_FILE);
    catched = true;
  }
  ASSERT_TRUE(catched);
}

}  // hrd
}  // loader
}  // motis
