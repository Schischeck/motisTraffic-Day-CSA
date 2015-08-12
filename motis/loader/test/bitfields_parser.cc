#include "catch/catch.hpp"

#include "parser/cstr.h"

#include "motis/loader/parser_error.h"
#include "motis/loader/parsers/hrd/bitfields_parser.h"

using namespace parser;

namespace motis {
namespace loader {
namespace hrd {

TEST_CASE("to_bitset_invalid_period_no_1") {
  bool catched = false;
  try {
    to_bitset("0", "file.101", 1);
  } catch (parser_error const& e) {
    catched = true;
  }
  REQUIRE(catched);
}

TEST_CASE("to_bitset_invalid_period_one_1") {
  bool catched = false;
  try {
    to_bitset("1", "file.101", 1);
  } catch (parser_error const& e) {
    catched = true;
  }
  REQUIRE(catched);
}

TEST_CASE("to_bitset_invalid_period_two_1") {
  bool catched = false;
  try {
    to_bitset("3", "file.101", 1);
  } catch (parser_error const& e) {
    catched = true;
  }
  REQUIRE(catched);
}

TEST_CASE("to_bitset_invalid_period_three_1") {
  bool catched = false;
  try {
    to_bitset("7", "file.101", 1);
  } catch (parser_error const& e) {
    catched = true;
  }
  REQUIRE(catched);
}

TEST_CASE("to_bitset_invalid_period_four_1") {
  bool catched = false;
  try {
    to_bitset("F", "file.101", 1);
  } catch (parser_error const& e) {
    catched = true;
  }
  REQUIRE(catched);
}

TEST_CASE("to_bitset_valid_period_1") {
  // 0x0653 = 0000 0110 0101 0011
  REQUIRE(std::bitset<BIT_COUNT>("0010100") ==
          to_bitset("0653", "file.101", 1));
}

TEST_CASE("to_bitset_valid_period_2") {
  // 0xC218 = 1100 0010 0001 1000
  REQUIRE(std::bitset<BIT_COUNT>("000010000") ==
          to_bitset("C218", "file.101", 1));
}

}  // hrd
}  // loader
}  // motis
