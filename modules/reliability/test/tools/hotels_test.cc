#include <algorithm>

#include "gtest/gtest.h"

#include "motis/reliability/tools/hotels.h"

using namespace motis::reliability;

TEST(reliability_hotels, parse) {
  auto const eva_nrs =
      hotels::parse_hotels("modules/reliability/resources/hotels.csv");
  ASSERT_FALSE(eva_nrs.empty());
  ASSERT_NE(eva_nrs.end(),
            std::find(eva_nrs.begin(), eva_nrs.end(), "8000013"));
}
