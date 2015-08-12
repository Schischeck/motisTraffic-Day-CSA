#include <cinttypes>
#include <cstring>

#include "catch/catch.hpp"

#include "parser/cstr.h"

#include "motis/loader/parser_error.h"
#include "motis/loader/parsers/hrd/files.h"
#include "motis/loader/parsers/hrd/attributes_parser.h"
#include "motis/loader/util.h"
#include "motis/schedule-format/Schedule_generated.h"

using namespace parser;

namespace motis {
namespace loader {
namespace hrd {

TEST_CASE("parse_line") {
  cstr file_content = ",  0 260 10 Bus mit Fahrradanhänger#";

  flatbuffers::FlatBufferBuilder b;

  auto attributes_map = parse_attributes({ATTRIBUTES_FILE, file_content}, b);

  REQUIRE(attributes_map.find(string_to_int<uint16_t>(", ")) !=
          attributes_map.end());

  b.Finish(CreateSchedule(b, {}, {}, b.CreateVector(values(attributes_map)), {},
                          {}));

  auto schedule = GetSchedule(b.GetBufferPointer());
  auto attributes_serialized = schedule->attributes();

  REQUIRE(attributes_serialized->size() == 1);
  REQUIRE(attributes_serialized->Get(0)->code()->str() == ", ");
  REQUIRE(attributes_serialized->Get(0)->text()->str() ==
          "Bus mit Fahrradanhänger");
}

TEST_CASE("parse_and_ignore_line") {
  cstr file_content = "ZZ 0 060 10 zusätzlicher Zug#\n# ,  ,  ,";

  flatbuffers::FlatBufferBuilder b;

  auto attributes_map = parse_attributes({ATTRIBUTES_FILE, file_content}, b);

  REQUIRE(attributes_map.find(string_to_int<uint16_t>("ZZ")) !=
          attributes_map.end());

  b.Finish(CreateSchedule(b, {}, {}, b.CreateVector(values(attributes_map)), {},
                          {}));

  auto schedule = GetSchedule(b.GetBufferPointer());
  auto attributes = schedule->attributes();

  REQUIRE(attributes->size() == 1);
  REQUIRE(attributes->Get(0)->code()->str() == "ZZ");
  REQUIRE(attributes->Get(0)->text()->str() == "zusätzlicher Zug");
}

TEST_CASE("invalid_line") {
  cstr file_content = ",  0 260 10 ";
  flatbuffers::FlatBufferBuilder b;

  bool catched = false;
  try {
    parse_attributes({ATTRIBUTES_FILE, file_content}, b);
  } catch (parser_error const& e) {
    catched = true;
    REQUIRE(strcmp(e.filename, ATTRIBUTES_FILE) == 0);
    REQUIRE(e.line_number == 1);
  }

  REQUIRE(catched);
}

TEST_CASE("ignore_output_rules") {
  cstr file_content = "# ,  ,  ,";
  flatbuffers::FlatBufferBuilder b;
  b.Finish(CreateSchedule(b, {}, {}, b.CreateVector(values(parse_attributes(
                                         {ATTRIBUTES_FILE, file_content}, b))),
                          {}, {}));

  auto schedule = GetSchedule(b.GetBufferPointer());
  auto attributes = schedule->attributes();

  REQUIRE(attributes->size() == 0);
}

}  // hrd
}  // loader
}  // motis
