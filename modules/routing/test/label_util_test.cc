#include "gtest/gtest.h"

#include "motis/routing/label_util.h"

namespace motis {
namespace label_util {

TEST(routing_label_util, night_travel_duration) {
  // 0 at days
  ASSERT_EQ(0, night_travel_duration(1000, 1200, false, 60, 359));
  ASSERT_EQ(0, night_travel_duration(1440 + 1000, 1440 + 1200, false, 60, 359));

  // 0 for hotels
  ASSERT_EQ(0, night_travel_duration(1000, 1200, true, 60, 359));
  ASSERT_EQ(0, night_travel_duration(100, 150, true, 60, 359));
  ASSERT_EQ(0, night_travel_duration(0, 150, true, 60, 359));
  ASSERT_EQ(0, night_travel_duration(100, 700, true, 60, 359));
  ASSERT_EQ(0, night_travel_duration(1440, 2000, true, 60, 359));

  // test intersections
  ASSERT_EQ(100, night_travel_duration(100, 200, false, 60, 359));
  ASSERT_EQ(40, night_travel_duration(0, 100, false, 60, 359));
  ASSERT_EQ(59, night_travel_duration(300, 400, false, 60, 359));
  ASSERT_EQ(299, night_travel_duration(60, 359, false, 60, 359));
  ASSERT_EQ(299, night_travel_duration(0, 400, false, 60, 359));

  ASSERT_EQ(100, night_travel_duration(1440 + 100, 1440 + 200, false, 60, 359));
  ASSERT_EQ(40, night_travel_duration(1420, 1440 + 100, false, 60, 359));
  ASSERT_EQ(59, night_travel_duration(1440 + 300, 1440 + 400, false, 60, 359));
  ASSERT_EQ(299, night_travel_duration(1440 + 60, 1440 + 359, false, 60, 359));
  ASSERT_EQ(299, night_travel_duration(1200, 1440 + 400, false, 60, 359));
}
}
}
