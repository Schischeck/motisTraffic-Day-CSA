#include "motis/loader/parsers/hrd/attributes_parser.h"

#include "catch/catch.hpp"

namespace motis {
namespace loader {
namespace hrd {

TEST_CASE("parse_line") {
  cstr file_content = ",  0 260 10 Bus mit Fahrradanhänger#";
  flatbuffers::FlatBufferBuilder builder;
  auto attributes = parse_attributes(builder, file_content);
  REQUIRE(attributes.size() == 1);
  REQUIRE(s.len == 0);
}

}  // hrd
}  // loader
}  // motis
