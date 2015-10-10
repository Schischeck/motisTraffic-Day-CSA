#include "gtest/gtest.h"

#include "motis/loader/parsers/gtfs/calendar.h"
#include "motis/loader/parsers/gtfs/files.h"

using namespace parser;

using namespace motis::loader::gtfs;

cstr example_calendar_file_content =
    R"(service_id,monday,tuesday,wednesday,thursday,friday,saturday,sunday,start_date,end_date
WE,0,0,0,0,0,1,1,20060701,20060731
WD,1,1,1,1,1,0,0,20060701,20060731)";

TEST(loader_gtfs_calendar, read_calendar_example_data) {
  auto calendar = read_calendar({CALENDAR_FILE, example_calendar_file_content});

  EXPECT_EQ(2, calendar.size());

  EXPECT_FALSE(calendar["WE"].week_days.test(0));
  EXPECT_FALSE(calendar["WE"].week_days.test(1));
  EXPECT_FALSE(calendar["WE"].week_days.test(2));
  EXPECT_FALSE(calendar["WE"].week_days.test(3));
  EXPECT_FALSE(calendar["WE"].week_days.test(4));
  EXPECT_TRUE(calendar["WE"].week_days.test(5));
  EXPECT_TRUE(calendar["WE"].week_days.test(6));

  EXPECT_TRUE(calendar["WD"].week_days.test(0));
  EXPECT_TRUE(calendar["WD"].week_days.test(1));
  EXPECT_TRUE(calendar["WD"].week_days.test(2));
  EXPECT_TRUE(calendar["WD"].week_days.test(3));
  EXPECT_TRUE(calendar["WD"].week_days.test(4));
  EXPECT_FALSE(calendar["WD"].week_days.test(5));
  EXPECT_FALSE(calendar["WD"].week_days.test(6));

  EXPECT_EQ(20060701, calendar["WE"].first_day);
  EXPECT_EQ(20060731, calendar["WE"].last_day);
  EXPECT_EQ(20060701, calendar["WD"].first_day);
  EXPECT_EQ(20060731, calendar["WD"].last_day);
}