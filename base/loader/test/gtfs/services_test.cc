#include "gtest/gtest.h"

#include "motis/loader/gtfs/calendar_date.h"
#include "motis/loader/gtfs/calendar.h"
#include "motis/loader/gtfs/services.h"
#include "motis/loader/gtfs/files.h"

#include "./resources.h"

using namespace parser;
using namespace motis::loader;
using namespace motis::loader::gtfs;

/*
 * -- BASE --
 * WE:       11         WD:       00
 *      0000011              1111100
 *      0000011              1111100
 *      0000011              1111100
 *      0000011              1111100
 *      0                    1
 */

/*
 * -- DATES --
 * WE:       11         WD:       00
 *      1100011              0011100
 *      0000011              1111100
 *      0000011              1111100
 *      0000011              1111100
 *      0                    1
 */

TEST(loader_gtfs_traffic_days, read_traffic_days_example_data) {
  auto dates = read_calendar_date(
      loaded_file{SCHEDULES / "example" / CALENDAR_DATES_FILE});
  auto calendar =
      read_calendar(loaded_file{SCHEDULES / "example" / CALENDAR_FILE});
  auto services = traffic_days(calendar, dates);

  std::string WE_bit_str = "1111000110000011000001100000110";
  std::string WD_bit_str = "0000111001111100111110011111001";
  std::reverse(begin(WE_bit_str), end(WE_bit_str));
  std::reverse(begin(WD_bit_str), end(WD_bit_str));
  bitfield WE_traffic_days(WE_bit_str);
  bitfield WD_traffic_days(WD_bit_str);

  EXPECT_EQ(boost::gregorian::date(2006, 7, 1), services.first_day);
  EXPECT_EQ(boost::gregorian::date(2006, 7, 31), services.last_day);

  EXPECT_EQ(WE_traffic_days, *services.traffic_days["WE"]);
  EXPECT_EQ(WD_traffic_days, *services.traffic_days["WD"]);
}
