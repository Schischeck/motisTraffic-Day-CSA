#include "gtest/gtest.h"

#include "motis/loader/gtfs/stop_time.h"
#include "motis/loader/gtfs/files.h"
#include "./test_files.h"

using namespace parser;

namespace motis {
namespace loader {
namespace gtfs {

TEST(loader_gtfs_route, read_stop_times_example_data) {
  auto agencies = read_agencies({AGENCY_FILE, example_agencies_file_content});
  auto routes =
      read_routes({ROUTES_FILE, example_routes_file_content}, agencies);
  auto dates = read_calendar_date(
      {CALENDAR_DATES_FILE, example_calendar_date_file_content});
  auto calendar = read_calendar({CALENDAR_FILE, example_calendar_file_content});
  auto services = traffic_days(calendar, dates);
  auto trips =
      read_trips({TRIPS_FILE, example_trips_file_content}, routes, services);
  auto stops = read_stops({STOPS_FILE, example_stops_file_content});
  read_stop_times({STOP_TIMES_FILE, example_stop_times_file_content}, trips,
                  stops);

  auto awe1_it = trips.find("AWE1");
  ASSERT_NE(end(trips), awe1_it);

  auto& awe1_stops = awe1_it->second->stop_times_;
  auto& stop = awe1_stops[1];
  EXPECT_EQ("S1", stop.stop_->id_);
  EXPECT_EQ(6, stop.arr_.time_);
  EXPECT_EQ(6, stop.dep_.time_);
  EXPECT_TRUE(stop.arr_.in_out_allowed_);
  EXPECT_TRUE(stop.dep_.in_out_allowed_);

  stop = awe1_stops[2];
  EXPECT_EQ("S2", stop.stop_->id_);
  EXPECT_EQ(-1, stop.arr_.time_);
  EXPECT_EQ(-1, stop.dep_.time_);
  EXPECT_FALSE(stop.arr_.in_out_allowed_);
  EXPECT_TRUE(stop.dep_.in_out_allowed_);

  stop = awe1_stops[3];
  EXPECT_EQ("S3", stop.stop_->id_);
  EXPECT_EQ(6, stop.arr_.time_);
  EXPECT_EQ(6, stop.dep_.time_);
  EXPECT_TRUE(stop.arr_.in_out_allowed_);
  EXPECT_TRUE(stop.dep_.in_out_allowed_);

  stop = awe1_stops[4];
  EXPECT_EQ("S5", stop.stop_->id_);
  EXPECT_EQ(-1, stop.arr_.time_);
  EXPECT_EQ(-1, stop.dep_.time_);
  EXPECT_TRUE(stop.arr_.in_out_allowed_);
  EXPECT_TRUE(stop.dep_.in_out_allowed_);

  stop = awe1_stops[5];
  EXPECT_EQ("S6", stop.stop_->id_);
  EXPECT_EQ(6, stop.arr_.time_);
  EXPECT_EQ(6, stop.dep_.time_);
  EXPECT_TRUE(stop.arr_.in_out_allowed_);
  EXPECT_TRUE(stop.dep_.in_out_allowed_);

  //"AWD1"
  // case 1:
  //   EXPECT_EQ("S1", stop.stop_->id_);
  //   EXPECT_EQ(6, stop.arr_.time_);
  //   EXPECT_EQ(6, stop.dep_.time_);
  //   EXPECT_TRUE(stop.arr_.in_out_allowed_);
  //   EXPECT_TRUE(stop.dep_.in_out_allowed_);
  //   break;
  // case 2:
  //   EXPECT_EQ("S2", stop.stop_->id_);
  //   EXPECT_EQ(-1, stop.arr_.time_);
  //   EXPECT_EQ(-1, stop.dep_.time_);
  //   EXPECT_TRUE(stop.arr_.in_out_allowed_);
  //   EXPECT_TRUE(stop.dep_.in_out_allowed_);
  //   break;
  // case 3:
  //   EXPECT_EQ("S3", stop.stop_->id_);
  //   EXPECT_EQ(6, stop.arr_.time_);
  //   EXPECT_EQ(6, stop.dep_.time_);
  //   EXPECT_TRUE(stop.arr_.in_out_allowed_);
  //   EXPECT_TRUE(stop.dep_.in_out_allowed_);
  //   break;
  // case 4:
  //   EXPECT_EQ("S4", stop.stop_->id_);
  //   EXPECT_EQ(-1, stop.arr_.time_);
  //   EXPECT_EQ(-1, stop.dep_.time_);
  //   EXPECT_TRUE(stop.arr_.in_out_allowed_);
  //   EXPECT_TRUE(stop.dep_.in_out_allowed_);
  //   break;
  // case 5:
  //   EXPECT_EQ("S5", stop.stop_->id_);
  //   EXPECT_EQ(-1, stop.arr_.time_);
  //   EXPECT_EQ(-1, stop.dep_.time_);
  //   EXPECT_TRUE(stop.arr_.in_out_allowed_);
  //   EXPECT_TRUE(stop.dep_.in_out_allowed_);
  //   break;
  // case 6:
  //   EXPECT_EQ("S6", stop.stop_->id_);
  //   EXPECT_EQ(6, stop.arr_.time_);
  //   EXPECT_EQ(6, stop.dep_.time_);
  //   EXPECT_TRUE(stop.arr_.in_out_allowed_);
  //   EXPECT_TRUE(stop.dep_.in_out_allowed_);
  //   break;
}

/*
TEST(loader_gtfs_route, read_stop_time_s_berlin_data) {
  auto stop_time_s =
      read_stop_time_s({STOP_TIMES_FILE, berlin_stop_time_s_file_content});

  for (auto const& trip : stop_time_s) {
    EXPECT_TRUE(trip.first == "1" || trip.first == "2" || trip.first == "3");

    for (auto const& stop : trip) {
      if (trip.first == "1") {
        switch (stop.first) {
          case 1:
            EXPECT_EQ("9230999", stop.stop_->id_);
            EXPECT_EQ(4 * 60 + 45, stop.arr_.time_);
            EXPECT_EQ(4 * 60 + 46, stop.dep_.time_);
            EXPECT_FALSE(stop.arr_.in_out_allowed_);
            EXPECT_TRUE(stop.dep_.in_out_allowed_);
            break;
          case 4:
            EXPECT_EQ("9220070", stop.stop_->id_);
            EXPECT_EQ(5 * 60 + 6, stop.arr_.time_);
            EXPECT_EQ(5 * 60 + 6, stop.dep_.time_);
            EXPECT_FALSE(stop.arr_.in_out_allowed_);
            EXPECT_TRUE(stop.dep_.in_out_allowed_);
            break;
          case 7:
            EXPECT_EQ("9260024", stop.stop_->id_);
            EXPECT_EQ(5 * 60 + 32, stop.arr_.time_);
            EXPECT_EQ(5 * 60 + 32, stop.dep_.time_);
            EXPECT_TRUE(stop.arr_.in_out_allowed_);
            EXPECT_FALSE(stop.dep_.in_out_allowed_);
            break;
        }
      } else if (trip.first == "2") {
        switch (stop.first) {
          case 1:
            EXPECT_EQ("9260024", stop.stop_->id_);
            EXPECT_EQ(5 * 60 + 35, stop.arr_.time_);
            EXPECT_EQ(5 * 60 + 35, stop.dep_.time_);
            EXPECT_FALSE(stop.arr_.in_out_allowed_);
            EXPECT_TRUE(stop.dep_.in_out_allowed_);
            break;
          case 4:
            EXPECT_EQ("9220070", stop.stop_->id_);
            EXPECT_EQ(6 * 60 + 2, stop.arr_.time_);
            EXPECT_EQ(6 * 60 + 3, stop.dep_.time_);
            EXPECT_TRUE(stop.arr_.in_out_allowed_);
            EXPECT_FALSE(stop.dep_.in_out_allowed_);
            break;
          case 7:
            EXPECT_EQ("9230999", stop.stop_->id_);
            EXPECT_EQ(6 * 60 + 22, stop.arr_.time_);
            EXPECT_EQ(6 * 60 + 22, stop.dep_.time_);
            EXPECT_TRUE(stop.arr_.in_out_allowed_);
            EXPECT_FALSE(stop.dep_.in_out_allowed_);
            break;
        }
      } else if (trip.first == "3") {
        switch (stop.first) {
          case 9:
            EXPECT_EQ("9222088", stop.stop_->id_);
            EXPECT_EQ(7 * 60 + 11, stop.arr_.time_);
            EXPECT_EQ(7 * 60 + 11, stop.dep_.time_);
            EXPECT_TRUE(stop.arr_.in_out_allowed_);
            EXPECT_FALSE(stop.dep_.in_out_allowed_);
            break;
        }
      }
    }
  }
}
*/

}  // gtfs
}  // loader
}  // motis
