#include "gtest/gtest.h"

#include "motis/loader/gtfs/calendar_date.h"
#include "motis/loader/gtfs/calendar.h"
#include "motis/loader/gtfs/traffic_days.h"
#include "motis/loader/gtfs/files.h"

using namespace parser;

namespace motis {
namespace loader {
namespace gtfs {

const char* example_calendar_date_file_content =
    R"(service_id,date,exception_type
WD,20060703,2
WE,20060703,1
WD,20060704,2
WE,20060704,1)";

const char* example_calendar_file_content =
    R"(service_id,monday,tuesday,wednesday,thursday,friday,saturday,sunday,start_date,end_date
WE,0,0,0,0,0,1,1,20060701,20060731
WD,1,1,1,1,1,0,0,20060701,20060731)";

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
      {CALENDAR_DATES_FILE, example_calendar_date_file_content});
  auto calendar = read_calendar({CALENDAR_FILE, example_calendar_file_content});
  auto services = traffic_days(calendar, dates);

  std::string WE_bit_str = "1111000110000011000001100000110";
  std::string WD_bit_str = "0000111001111100111110011111001";
  std::reverse(begin(WE_bit_str), end(WE_bit_str));
  std::reverse(begin(WD_bit_str), end(WD_bit_str));
  bitfield WE_traffic_days(WE_bit_str);
  bitfield WD_traffic_days(WD_bit_str);

  EXPECT_EQ(boost::gregorian::date(2006, 7, 1), services.first_day);
  EXPECT_EQ(boost::gregorian::date(2006, 7, 31), services.last_day);

  EXPECT_EQ(WE_traffic_days, services.traffic_days["WE"]);
  EXPECT_EQ(WD_traffic_days, services.traffic_days["WD"]);
}

}  // gtfs
}  // loader
}  // motis
