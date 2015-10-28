#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

#if 0

#include "test_schedule.h"

#include <iostream>

TEST_CASE("load test schedule") {
  std::cout << "loading schedule...\n";
  motis::realtime::test::test_schedule ts;
  std::cout << "schedule loaded\n";
}

#endif