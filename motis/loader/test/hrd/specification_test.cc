#include <cinttypes>
#include <cstring>
#include "catch/catch.hpp"

#include "test_spec.h"

#include "parser/cstr.h"
#include "parser/util.h"

#include "motis/loader/parser_error.h"

using namespace parser;
using namespace flatbuffers;
namespace fs = boost::filesystem;

namespace motis {
namespace loader {
namespace hrd {

TEST_CASE("parse_specification") {
  const auto fahrten = SCHEDULES / "hand-crafted" / "fahrten";
  test_spec services_file(fahrten, "services-1.101");

  const auto specs = services_file.get_specs();
  REQUIRE(specs.size() == 1);

  auto& spec = specs[0];
  REQUIRE(!spec.is_empty());
  REQUIRE(spec.valid());
  REQUIRE(!spec.internal_service.empty());
  REQUIRE(spec.traffic_days.size() == 1);
  REQUIRE(spec.categories.size() == 1);
  REQUIRE(spec.attributes.size() == 3);
  REQUIRE(spec.stops.size() == 6);
}

void require_parser_err(char const* schedule_name, char const* filename) {}

TEST_CASE("parse_hrd_service_invalid_traffic_days") {
  bool catched = false;
  try {
    test_spec(SCHEDULES / "hand-crafted" / "fahrten", "services-3.101")
        .get_services();
  } catch (std::runtime_error const& e) {
    catched = true;
  }
  REQUIRE(catched);
}

}  // hrd
}  // loader
}  // motis
