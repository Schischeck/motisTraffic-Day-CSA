#include "gtest/gtest.h"

#include "parser/cstr.h"

#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/parsers/hrd/bitfields_parser.h"

#include "../convert.h"

using namespace parser;

namespace motis {
namespace loader {
namespace hrd {

TEST(loader_hrd_bitfields, to_bitset_invalid_period_no_1) {
  bool catched = false;
  try {
    hex_str_to_bitset("0", "file.101", 1);
  } catch (parser_error const& e) {
    catched = true;
  }
  ASSERT_TRUE(catched);
}

TEST(loader_hrd_bitfields, hex_str_to_bitset_invalid_period_one_1) {
  bool catched = false;
  try {
    hex_str_to_bitset("1", "file.101", 1);
  } catch (parser_error const& e) {
    catched = true;
  }
  ASSERT_TRUE(catched);
}

TEST(loader_hrd_bitfields, hex_str_to_bitset_invalid_period_two_1) {
  bool catched = false;
  try {
    hex_str_to_bitset("3", "file.101", 1);
  } catch (parser_error const& e) {
    catched = true;
  }
  ASSERT_TRUE(catched);
}

TEST(loader_hrd_bitfields, hex_str_to_bitset_invalid_period_three_1) {
  bool catched = false;
  try {
    hex_str_to_bitset("7", "file.101", 1);
  } catch (parser_error const& e) {
    catched = true;
  }
  ASSERT_TRUE(catched);
}

TEST(loader_hrd_bitfields, hex_str_to_bitset_invalid_period_four_1) {
  bool catched = false;
  try {
    hex_str_to_bitset("F", "file.101", 1);
  } catch (parser_error const& e) {
    catched = true;
  }
  ASSERT_TRUE(catched);
}

TEST(loader_hrd_bitfields, hex_str_to_bitset_valid_period_1) {
  // 0x0653 = 0000 0110 0101 0011
  ASSERT_TRUE(std::bitset<BIT_COUNT>("0010100") ==
              hex_str_to_bitset("0653", "file.101", 1));
}

TEST(loader_hrd_bitfields, hex_str_to_bitset_valid_period_2) {
  // 0xC218 = 1100 0010 0001 1000
  ASSERT_TRUE(std::bitset<BIT_COUNT>("000010000") ==
              hex_str_to_bitset("C218", "file.101", 1));
}

TEST(loader_hrd_bitfields, deserialize_bitset) {
  flatbuffers::FlatBufferBuilder b;

  auto serialized_bitfield = b.CreateString(
      serialize_bitset<BIT_COUNT>(std::bitset<BIT_COUNT>("000010000")));

  auto str = to_string(serialized_bitfield, b);
  std::bitset<512> bitset = deserialize_bitset<BIT_COUNT>(str.c_str());
  ASSERT_TRUE(std::bitset<BIT_COUNT>("000010000") == bitset);
}

}  // hrd
}  // loader
}  // motis
