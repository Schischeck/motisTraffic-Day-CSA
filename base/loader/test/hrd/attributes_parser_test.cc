#include <cinttypes>
#include <cstring>

#include "gtest/gtest.h"

#include "motis/loader/hrd/parser/attributes_parser.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/util.h"

using namespace parser;

namespace motis {
namespace loader {
namespace hrd {

TEST(loader_hrd_attributes, parse_line) {
  auto const c = hrd_5_00_8_;
  loaded_file f = {c.files(ATTRIBUTES), ",  0 260 10 Bus mit Fahrradanhänger#"};
  auto attributes = parse_attributes(f, c);
  ASSERT_TRUE(attributes.size() == 1);

  auto it = attributes.find(raw_to_int<uint16_t>(", "));
  ASSERT_TRUE(it != end(attributes));
  ASSERT_TRUE(it->second == "Bus mit Fahrradanhänger");
}

TEST(loader_hrd_attributes, parse_and_ignore_line) {
  auto const c = hrd_5_00_8_;
  loaded_file f = {c.files(ATTRIBUTES),
                   "ZZ 0 060 10 zusätzlicher Zug#\n# ,  ,  ,"};
  auto attributes = parse_attributes(f, c);
  ASSERT_TRUE(attributes.size() == 1);

  auto it = attributes.find(raw_to_int<uint16_t>("ZZ"));
  ASSERT_TRUE(it != end(attributes));
  ASSERT_TRUE(it->second == "zusätzlicher Zug");
}

TEST(loader_hrd_attributes, invalid_line) {
  auto const c = hrd_5_00_8_;
  bool catched = false;
  loaded_file f = {c.files(ATTRIBUTES), ",  0 260 10 "};
  try {
    parse_attributes(f, c);
  } catch (parser_error const& e) {
    catched = true;
    ASSERT_STREQ(c.files(ATTRIBUTES), e.filename_);
    ASSERT_TRUE(e.line_number_ == 1);
  }

  ASSERT_TRUE(catched);
}

TEST(loader_hrd_attributes, ignore_output_rules) {
  auto const c = hrd_5_00_8_;
  loaded_file f = {c.files(ATTRIBUTES), "# ,  ,  ,"};
  ASSERT_TRUE(parse_attributes(f, c).empty());
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
