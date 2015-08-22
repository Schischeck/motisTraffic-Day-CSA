#define CATCH_CONFIG_MAIN

#include "motis/loader/loader.h"

#include "motis/reliability/reliability.h"

#include "catch/catch.hpp"

using namespace motis::reliability;

TEST_CASE("Initial distributions are calculated", "[initial]") {
  auto schedule = motis::loader::load_schedule("../schedule/test");
}
