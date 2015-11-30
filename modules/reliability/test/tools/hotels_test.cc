#include <algorithm>

#include "gtest/gtest.h"

#include "motis/reliability/tools/hotels.h"

using namespace motis::reliability;

TEST(reliability_hotels, parse) {
  auto const hotels =
      hotels::parse_hotels("modules/reliability/resources/hotels.csv");
  ASSERT_FALSE(hotels.empty());
  ASSERT_NE(hotels.end(), std::find_if(hotels.begin(), hotels.end(),
                                       [](hotels::hotel_info const& info) {
                                         return info.station_ == "8000013";
                                       }));
}
