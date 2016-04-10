#include <cinttypes>
#include <cstring>

#include "gtest/gtest.h"

#include "motis/loader/hrd/files.h"
#include "motis/loader/hrd/parser/attributes_parser.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/util.h"

using namespace parser;

namespace motis {
namespace loader {
namespace hrd {

TEST(loader_hrd_attributes, parse_line) {
  loaded_file f = {ATTRIBUTES_FILE_OLD, ",  0 260 10 Bus mit Fahrradanhänger#"};
  auto attributes = parse_attributes(f);
  ASSERT_TRUE(attributes.size() == 1);

  auto it = attributes.find(raw_to_int<uint16_t>(", "));
  ASSERT_TRUE(it != end(attributes));
  ASSERT_TRUE(it->second == "Bus mit Fahrradanhänger");
}

TEST(loader_hrd_attributes, parse_and_ignore_line) {
  loaded_file f = {ATTRIBUTES_FILE_OLD,
                   "ZZ 0 060 10 zusätzlicher Zug#\n# ,  ,  ,"};
  auto attributes = parse_attributes(f);
  ASSERT_TRUE(attributes.size() == 1);

  auto it = attributes.find(raw_to_int<uint16_t>("ZZ"));
  ASSERT_TRUE(it != end(attributes));
  ASSERT_TRUE(it->second == "zusätzlicher Zug");
}

TEST(loader_hrd_attributes, invalid_line) {
  bool catched = false;
  loaded_file f = {ATTRIBUTES_FILE_OLD, ",  0 260 10 "};
  try {
    parse_attributes(f);
  } catch (parser_error const& e) {
    catched = true;
    ASSERT_STREQ(ATTRIBUTES_FILE_OLD, e.filename_);
    ASSERT_TRUE(e.line_number_ == 1);
  }

  ASSERT_TRUE(catched);
}

TEST(loader_hrd_attributes, ignore_output_rules) {
  loaded_file f = {ATTRIBUTES_FILE_OLD, "# ,  ,  ,"};
  ASSERT_TRUE(parse_attributes(f).size() == 0);
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
