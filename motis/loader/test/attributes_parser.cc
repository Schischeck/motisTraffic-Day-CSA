#define CATCH_CONFIG_MAIN

#include "catch/catch.hpp"

#include "motis/loader/parsers/hrd/attributes_parser.h"
#include "motis/schedule-format/Schedule_generated.h"

using namespace parser;

namespace motis {
namespace loader {
namespace hrd {

TEST_CASE("parse_line") {
  cstr file_content = ",  0 260 10 Bus mit Fahrradanhänger#";

  flatbuffers::FlatBufferBuilder b;
  b.Finish(CreateSchedule(
      b, {}, {}, b.CreateVector(parse_attributes(b, file_content)), {}, {}));

  auto schedule = GetSchedule(b.GetBufferPointer());
  auto attributes = schedule->attributes();

  REQUIRE(attributes->size() == 1);
  REQUIRE(attributes->Get(0)->code()->str() == ", ");
  REQUIRE(attributes->Get(0)->text()->str() == "Bus mit Fahrradanhänger");
}

}  // hrd
}  // loader
}  // motis
