#include <cinttypes>
#include <cstring>
#include <bitset>

#include "gtest/gtest.h"

#include "parser/arg_parser.h"
#include "parser/cstr.h"

#include "motis/loader/hrd/files.h"
#include "motis/loader/hrd/parser/bitfields_parser.h"
#include "motis/loader/hrd/parser/track_rules_parser.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/util.h"
#include "motis/schedule-format/Schedule_generated.h"

using namespace parser;

namespace motis {
namespace loader {
namespace hrd {

TEST(loader_hrd_track_rules, parse_track_rules_1) {
  flatbuffers64::FlatBufferBuilder b;

  loaded_file bitfields_file = {BITFIELDS_FILE, "000001 EF"};
  auto track_file_content =
      "8509404 30467 85____ 3             000000\n"
      "8509404 30467 85____ 5             000001";
  loaded_file track_file = {TRACKS_FILE, track_file_content};

  auto bitfields = parse_bitfields(bitfields_file);
  auto track_rules = parse_track_rules(track_file, b);

  ASSERT_TRUE(track_rules.size() == 1);

  auto key = std::make_tuple(8509404, 30467, raw_to_int<uint64_t>("85____"));
  auto entry = track_rules.find(key);

  ASSERT_TRUE(entry != end(track_rules));

  auto rule_set = entry->second;

  ASSERT_TRUE(rule_set.size() == 2);
  // TODO(Felix Guendling)
  // ASSERT_TRUE(cstr(to_string(rule_set[0].track_name, b).c_str()) == "3");

  std::string all_days_bit_str;
  all_days_bit_str.resize(BIT_COUNT);
  std::fill(begin(all_days_bit_str), end(all_days_bit_str), '1');
  std::bitset<BIT_COUNT> all_days(all_days_bit_str);

  ASSERT_TRUE(rule_set[0].bitfield_num_ == 0);
  ASSERT_TRUE(rule_set[0].time_ == TIME_NOT_SET);

  // TODO(Felix Guendling)
  // ASSERT_TRUE(cstr(to_string(rule_set[1].track_name, b).c_str()) == "5");
  ASSERT_TRUE(rule_set[1].bitfield_num_ == 1);
  ASSERT_TRUE(rule_set[1].time_ == TIME_NOT_SET);
}

TEST(loader_hrd_track_rules, parse_track_rules_2) {
  flatbuffers64::FlatBufferBuilder b;

  loaded_file bitfields_file = {BITFIELDS_FILE, "000001 FF"};
  auto track_file_content = "8000000 00001 80____ 1A       0130 000001";
  loaded_file track_file = {TRACKS_FILE, track_file_content};

  auto bitfields = parse_bitfields(bitfields_file);
  auto track_rules = parse_track_rules(track_file, b);

  ASSERT_TRUE(track_rules.size() == 1);

  auto key = std::make_tuple(8000000, 1, raw_to_int<uint64_t>("80____"));
  auto entry = track_rules.find(key);

  ASSERT_TRUE(entry != track_rules.end());

  auto rule_set = entry->second;

  ASSERT_TRUE(rule_set.size() == 1);

  // 800000 00001 80____ 1A       0130 000001->[...01111 == (0xFF)]
  // TODO(Felix Guendling)
  // ASSERT_TRUE(cstr(to_string(rule_set[0].track_name, b).c_str()) == "1A");
  ASSERT_TRUE(rule_set[0].bitfield_num_ == 1);
  ASSERT_TRUE(rule_set[0].time_ == 90);
}

TEST(loader_hrd_track_rules, parse_track_rules_line_too_short) {
  bool catched = false;

  loaded_file f = {BITFIELDS_FILE, "000001 EF"};
  auto track_file_content =
      "8509404 30467 85____ 3             000000\n"
      "8509404 30467 85____ 5             00000";
  loaded_file track_rules_file = {TRACKS_FILE, track_file_content};

  try {
    flatbuffers64::FlatBufferBuilder b;

    auto bitfields = parse_bitfields(f);
    auto track_rules = parse_track_rules(track_rules_file, b);
  } catch (parser_error const& e) {
    ASSERT_TRUE(e.line_number_ == 2);
    ASSERT_STREQ(TRACKS_FILE, e.filename_);
    catched = true;
  }
  ASSERT_TRUE(catched);
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
