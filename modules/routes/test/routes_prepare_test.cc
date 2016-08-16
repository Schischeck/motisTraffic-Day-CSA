#include "gtest/gtest.h"

#include "motis/routes/prepare/osm_util.h"

TEST(routes_prepare_test, sort_ways) {
  std::vector<int64_t> expected = {0, 1, 2, 3, 4, 6, 7, 8};
  std::vector<motis::routes::way> ways = {
      motis::routes::way(0, 0, 1), motis::routes::way(2, 2, 3),
      motis::routes::way(1, 1, 2), motis::routes::way(4, 4, 5),
      motis::routes::way(3, 3, 4), motis::routes::way(6, 6, 7),
      motis::routes::way(7, 7, 8), motis::routes::way(8, 8, 9)};
  auto result = sort_ways(ways);
  EXPECT_EQ(result, expected);
}
