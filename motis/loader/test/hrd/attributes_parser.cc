#include <cinttypes>
#include <cstring>

#include "catch/catch.hpp"

#include "parser/cstr.h"

#include "motis/loader/parser_error.h"
#include "motis/loader/parsers/hrd/files.h"
#include "motis/loader/parsers/hrd/attributes_parser.h"
#include "motis/loader/util.h"

using namespace parser;

namespace motis {
namespace loader {
namespace hrd {

TEST_CASE("parse_line") {
  cstr file_content = ",  0 260 10 Bus mit Fahrradanhänger#";
  auto attributes = parse_attributes({ATTRIBUTES_FILE, file_content});
  REQUIRE(attributes.size() == 1);

  auto it = attributes.find(raw_to_int<uint16_t>(", "));
  REQUIRE(it != end(attributes));
  REQUIRE(it->second == "Bus mit Fahrradanhänger");
}

TEST_CASE("parse_and_ignore_line") {
  cstr file_content = "ZZ 0 060 10 zusätzlicher Zug#\n# ,  ,  ,";

  auto attributes = parse_attributes({ATTRIBUTES_FILE, file_content});
  REQUIRE(attributes.size() == 1);

  auto it = attributes.find(raw_to_int<uint16_t>("ZZ"));
  REQUIRE(it != end(attributes));
  REQUIRE(it->second == "zusätzlicher Zug");
}

TEST_CASE("invalid_line") {
  cstr file_content = ",  0 260 10 ";

  bool catched = false;
  try {
    parse_attributes({ATTRIBUTES_FILE, file_content});
  } catch (parser_error const& e) {
    catched = true;
    REQUIRE(strcmp(e.filename, ATTRIBUTES_FILE) == 0);
    REQUIRE(e.line_number == 1);
  }

  REQUIRE(catched);
}

TEST_CASE("ignore_output_rules") {
  cstr file_content = "# ,  ,  ,";
  REQUIRE(parse_attributes({ATTRIBUTES_FILE, file_content}).size() == 0);
}

}  // hrd
}  // loader
}  // motis
