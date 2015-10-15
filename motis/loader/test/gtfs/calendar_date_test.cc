#include "gtest/gtest.h"

#include "motis/loader/parsers/gtfs/calendar_date.h"
#include "motis/loader/parsers/gtfs/files.h"

using namespace parser;

using namespace motis::loader::gtfs;

cstr example_calendar_date_file_content =
    R"(service_id,date,exception_type
WD,20060703,2
WE,20060703,1
WD,20060704,2
WE,20060704,1)";

cstr berlin_calendar_date_file_content =
    R"(service_id,date,exception_type
000001,20150409,1
000001,20150410,1
000001,20150411,1
000001,20150412,1
000001,20150705,1
000001,20150706,1
000001,20150707,1
000001,20150708,1
000001,20150709,1
000001,20150710,1
000001,20150711,1
000001,20150712,1
000001,20150713,1
000001,20150714,1
000002,20150715,1
000002,20150716,1
000002,20150717,1)";

TEST(loader_gtfs_calendar_date, read_calendar_date_example_data) {
  auto dates = read_calendar_date(
      {CALENDAR_DATES_FILE, example_calendar_date_file_content});

  EXPECT_EQ(2, dates.size());
  EXPECT_EQ(2, dates["WD"].size());
  EXPECT_EQ(2, dates["WE"].size());

  EXPECT_EQ(2006, dates["WD"][0].year);
  EXPECT_EQ(7, dates["WD"][0].month);
  EXPECT_EQ(3, dates["WD"][0].day);
  EXPECT_EQ(date::REMOVE, dates["WD"][0].type);

  EXPECT_EQ(2006, dates["WE"][0].year);
  EXPECT_EQ(7, dates["WE"][0].month);
  EXPECT_EQ(3, dates["WE"][0].day);
  EXPECT_EQ(date::ADD, dates["WE"][0].type);

  EXPECT_EQ(2006, dates["WD"][1].year);
  EXPECT_EQ(7, dates["WD"][1].month);
  EXPECT_EQ(4, dates["WD"][1].day);
  EXPECT_EQ(date::REMOVE, dates["WD"][1].type);

  EXPECT_EQ(2006, dates["WE"][1].year);
  EXPECT_EQ(7, dates["WE"][1].month);
  EXPECT_EQ(4, dates["WE"][1].day);
  EXPECT_EQ(date::ADD, dates["WE"][1].type);
}