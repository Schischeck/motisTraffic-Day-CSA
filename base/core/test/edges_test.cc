#include "gtest/gtest.h"

#include "motis/core/schedule/edges.h"

namespace motis {

TEST(routing_late_connections, time_off) {
  ASSERT_EQ(100, edge::calc_time_off(100, 200, 0));
  ASSERT_EQ(0, edge::calc_time_off(100, 200, 100));
  ASSERT_EQ(0, edge::calc_time_off(100, 200, 150));
  ASSERT_EQ(0, edge::calc_time_off(100, 200, 200));
  ASSERT_EQ(1240, edge::calc_time_off(100, 200, 300));
  ASSERT_EQ(101, edge::calc_time_off(100, 200, 1439));

  ASSERT_EQ(0, edge::calc_time_off(1340, 40, 0));
  ASSERT_EQ(0, edge::calc_time_off(1340, 40, 40));
  ASSERT_EQ(1200, edge::calc_time_off(1340, 40, 140));
  ASSERT_EQ(1, edge::calc_time_off(1340, 40, 1339));
  ASSERT_EQ(0, edge::calc_time_off(1340, 40, 1340));
  ASSERT_EQ(0, edge::calc_time_off(1340, 40, 1439));
}

}  // namespace motis
