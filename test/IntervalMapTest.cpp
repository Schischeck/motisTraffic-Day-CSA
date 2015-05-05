#include "gtest/gtest.h"

#include "IntervalMap.h"

using namespace td;

TEST(IntervalMapTest, SimpleTest)
{
  // ---INPUT---
  // Sections:
  // 1: [1-10]
  // 2: [11-20]
  //
  // Attributes:
  // ID | Section Based              | Simplified
  // -------------------------------------------------------
  // 1  | [8-10], [11-13], [19-20]   | [8-13], [19-20]
  // 2  | [1-20]                     | [1-20]
  // 3  | [1-5], [8-10], [10-15]     | [1-5], [8-15]
  // 4  | [1-5], [10-15], [6-9]      | [1-15]

  IntervalMap map;

  map.addEntry(1, 8, 10);
  map.addEntry(1, 11, 13);
  map.addEntry(1, 19, 20);

  map.addEntry(2, 1, 20);

  map.addEntry(3, 1, 5);
  map.addEntry(3, 8, 10);
  map.addEntry(3, 10, 15);

  map.addEntry(4, 1, 5);
  map.addEntry(4, 10, 15);
  map.addEntry(4, 6, 9);

  auto ranges = map.getAttributeRanges();

  EXPECT_EQ( 8, ranges[1][0].from);
  EXPECT_EQ(13, ranges[1][0].to);
  EXPECT_EQ(19, ranges[1][1].from);
  EXPECT_EQ(20, ranges[1][1].to);

  EXPECT_EQ( 1, ranges[2][0].from);
  EXPECT_EQ(20, ranges[2][0].to);

  EXPECT_EQ( 1, ranges[3][0].from);
  EXPECT_EQ( 5, ranges[3][0].to);
  EXPECT_EQ( 8, ranges[3][1].from);
  EXPECT_EQ(15, ranges[3][1].to);

  EXPECT_EQ( 1, ranges[4][0].from);
  EXPECT_EQ(15, ranges[4][0].to);
}
