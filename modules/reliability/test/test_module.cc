#define CATCH_CONFIG_MAIN

#include "catch/catch.hpp"

#include "motis/loader/loader.h"

#include "motis/reliability/reliability.h"

using namespace motis::reliability;

TEST_CASE("Initial distributions are calculated", "[initial]") {
  auto schedule = motis::load_schedule("../schedule/test");

  reliability rel;
  rel.schedule_ = schedule.get();
}
