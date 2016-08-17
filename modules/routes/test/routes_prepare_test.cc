#include "gtest/gtest.h"

#include "motis/routes/prepare/osm_util.h"

TEST(routes_prepare_test, sort_ways_normal) {
  std::vector<int64_t> expected = {0, 1, 2, 3, 4, 6, 7, 8};
  std::vector<motis::routes::way> ways = {
      motis::routes::way(0, 0, 1), motis::routes::way(2, 2, 3),
      motis::routes::way(1, 1, 2), motis::routes::way(4, 4, 5),
      motis::routes::way(3, 3, 4), motis::routes::way(6, 6, 7),
      motis::routes::way(7, 7, 8), motis::routes::way(8, 8, 9)};
  auto result = sort_ways(ways);
  EXPECT_EQ(expected, result);
}

TEST(routes_prepare_test, sort_ways_single) {
  std::vector<int64_t> expected = {0, 2, 1, 4};
  std::vector<motis::routes::way> ways = {
      motis::routes::way(0, 0, 0), motis::routes::way(2, 2, 2),
      motis::routes::way(1, 1, 1), motis::routes::way(4, 4, 5)};
  auto result = sort_ways(ways);
  EXPECT_EQ(expected, result);
}

TEST(routes_prepare_test, sort_ways_realdata) {
  std::vector<int64_t> expected = {
      54049525,  392552209, 219197793, 314977816, 259165449, 4393602, 405088368,
      316734228, 316734229, 23594455,  316734224, 259175856, 66208929};
  std::vector<motis::routes::way> ways = {
      motis::routes::way(54049525, 3957691516, 404616786),
      motis::routes::way(392552209, 404616786, 419331424),
      motis::routes::way(219197793, 419331424, 419331425),
      motis::routes::way(314977816, 419331425, 2645230906),
      motis::routes::way(259165449, 2645230906, 529182),
      motis::routes::way(4393602, 529182, 1832744777),
      motis::routes::way(405088368, 1832744777, 3228910915),
      motis::routes::way(316734228, 3228910915, 3228910914),
      motis::routes::way(316734229, 3228910914, 36428036),
      motis::routes::way(23594455, 36428036, 36427973),
      motis::routes::way(316734224, 36427973, 367272686),
      motis::routes::way(259175856, 367272686, 499146),
      motis::routes::way(66208929, 499146, 25270707)};
  auto result = sort_ways(ways);
  EXPECT_EQ(expected, result);
}
