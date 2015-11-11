#include <cinttypes>
#include <cstring>

#include "gtest/gtest.h"

#include "parser/cstr.h"
#include "parser/util.h"

#include "motis/loader/parser_error.h"

#include "./test_spec_test.h"

using namespace parser;
using namespace flatbuffers;
namespace fs = boost::filesystem;

namespace motis {
namespace loader {
namespace hrd {

TEST(loader_hrd_specification, parse_specification) {
  const auto fahrten = SCHEDULES / "hand-crafted" / "fahrten";
  test_spec services_file(fahrten, "services-1.101");

  const auto specs = services_file.get_specs();
  ASSERT_TRUE(specs.size() == 1);

  auto& spec = specs[0];
  ASSERT_TRUE(!spec.is_empty());
  ASSERT_TRUE(spec.valid());
  ASSERT_TRUE(!spec.internal_service.empty());
  ASSERT_TRUE(spec.traffic_days.size() == 1);
  ASSERT_TRUE(spec.categories.size() == 1);
  ASSERT_TRUE(spec.attributes.size() == 3);
  ASSERT_TRUE(spec.stops.size() == 6);
}

TEST(loader_hrd_specification, parse_hrd_service_invalid_traffic_days) {
  bool catched = false;
  try {
    test_spec(SCHEDULES / "hand-crafted" / "fahrten", "services-3.101")
        .get_hrd_services();
  } catch (std::runtime_error const& e) {
    catched = true;
  }
  ASSERT_TRUE(catched);
}

}  // hrd
}  // loader
}  // motis
