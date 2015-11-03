#include <cinttypes>
#include <cstring>

#include "gtest/gtest.h"

#include "motis/loader/parser_error.h"
#include "motis/loader/parsers/hrd/files.h"
#include "motis/loader/parsers/hrd/attributes_parser.h"
#include "motis/loader/util.h"

using namespace parser;

namespace motis {
namespace loader {
namespace hrd {

TEST(loader_hrd_attributes, parse_line) {
  auto attributes = parse_attributes(
      {ATTRIBUTES_FILE_OLD, ",  0 260 10 Bus mit Fahrradanhänger#"});
  ASSERT_TRUE(attributes.size() == 1);

  auto it = attributes.find(raw_to_int<uint16_t>(", "));
  ASSERT_TRUE(it != end(attributes));
  ASSERT_TRUE(it->second == "Bus mit Fahrradanhänger");
}

TEST(loader_hrd_attributes, parse_and_ignore_line) {
  auto attributes = parse_attributes(
      {ATTRIBUTES_FILE_OLD, "ZZ 0 060 10 zusätzlicher Zug#\n# ,  ,  ,"});
  ASSERT_TRUE(attributes.size() == 1);

  auto it = attributes.find(raw_to_int<uint16_t>("ZZ"));
  ASSERT_TRUE(it != end(attributes));
  ASSERT_TRUE(it->second == "zusätzlicher Zug");
}

TEST(loader_hrd_attributes, invalid_line) {
  bool catched = false;
  try {
    parse_attributes({ATTRIBUTES_FILE_OLD, ",  0 260 10 "});
  } catch (parser_error const& e) {
    catched = true;
    ASSERT_TRUE(strcmp(e.filename, ATTRIBUTES_FILE_OLD) == 0);
    ASSERT_TRUE(e.line_number == 1);
  }

  ASSERT_TRUE(catched);
}

TEST(loader_hrd_attributes, ignore_output_rules) {
  ASSERT_TRUE(parse_attributes({ATTRIBUTES_FILE_OLD, "# ,  ,  ,"}).size() == 0);
}

}  // hrd
}  // loader
}  // motis
