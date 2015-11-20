#include "gtest/gtest.h"

#include "motis/loader/gtfs/calendar.h"
#include "motis/loader/gtfs/files.h"
#include "./test_files.h"

using namespace parser;
using namespace motis::loader::gtfs;

TEST(loader_gtfs_calendar, read_calendar_example_data) {
  auto calendar = read_calendar({CALENDAR_FILE, example_calendar_file_content});

  EXPECT_EQ(2, calendar.size());

  EXPECT_TRUE(calendar["WE"].week_days.test(0));
  EXPECT_FALSE(calendar["WE"].week_days.test(1));
  EXPECT_FALSE(calendar["WE"].week_days.test(2));
  EXPECT_FALSE(calendar["WE"].week_days.test(3));
  EXPECT_FALSE(calendar["WE"].week_days.test(4));
  EXPECT_FALSE(calendar["WE"].week_days.test(5));
  EXPECT_TRUE(calendar["WE"].week_days.test(6));

  EXPECT_FALSE(calendar["WD"].week_days.test(0));
  EXPECT_TRUE(calendar["WD"].week_days.test(1));
  EXPECT_TRUE(calendar["WD"].week_days.test(2));
  EXPECT_TRUE(calendar["WD"].week_days.test(3));
  EXPECT_TRUE(calendar["WD"].week_days.test(4));
  EXPECT_TRUE(calendar["WD"].week_days.test(5));
  EXPECT_FALSE(calendar["WD"].week_days.test(6));

  EXPECT_EQ(boost::gregorian::date(2006, 07, 01), calendar["WE"].first_day);
  EXPECT_EQ(boost::gregorian::date(2006, 07, 31), calendar["WE"].last_day);
  EXPECT_EQ(boost::gregorian::date(2006, 07, 01), calendar["WD"].first_day);
  EXPECT_EQ(boost::gregorian::date(2006, 07, 31), calendar["WD"].last_day);
}
