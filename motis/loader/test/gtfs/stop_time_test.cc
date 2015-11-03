#include "gtest/gtest.h"

#include "motis/loader/parsers/gtfs/stop_time.h"
#include "motis/loader/parsers/gtfs/files.h"

using namespace parser;

namespace motis {
namespace loader {
namespace gtfs {

const char* example_stop_times_file_content =
    R"(trip_id,arrival_time,departure_time,stop_id,stop_sequence,pickup_type,drop_off_type
AWE1,0:06:10,0:06:10,S1,1,0,0,0
AWE1,,,S2,2,0,1,3
AWE1,0:06:20,0:06:30,S3,3,0,0,0
AWE1,,,S5,4,0,0,0
AWE1,0:06:45,0:06:45,S6,5,0,0,0
AWD1,0:06:10,0:06:10,S1,1,0,0,0
AWD1,,,S2,2,0,0,0
AWD1,0:06:20,0:06:20,S3,3,0,0,0
AWD1,,,S4,4,0,0,0
AWD1,,,S5,5,0,0,0
AWD1,0:06:45,0:06:45,S6,6,0,0,0)";

const char* berlin_stop_times_file_content =
    R"(trip_id,arrival_time,departure_time,stop_id,stop_sequence,stop_headsign,pickup_type,drop_off_type,shape_dist_traveled
1,04:45:00,04:46:00,9230999,1,,0,1,
1,04:51:00,04:51:00,9230400,2,,0,1,
1,04:59:00,04:59:00,9220019,3,,0,1,
1,05:06:00,05:06:00,9220070,4,,0,1,
1,05:11:00,05:11:00,9220114,5,,0,1,
1,05:13:00,05:13:00,9220001,6,,0,1,
1,05:32:00,05:32:00,9260024,7,,1,0,
2,05:35:00,05:35:00,9260024,1,,0,1,
2,05:54:00,05:54:00,9220001,2,,1,0,
2,05:56:00,05:56:00,9220114,3,,1,0,
2,06:02:00,06:03:00,9220070,4,,1,0,
2,06:08:00,06:08:00,9220019,5,,1,0,
2,06:15:00,06:15:00,9230400,6,,1,0,
2,06:22:00,06:22:00,9230999,7,,1,0,
3,06:53:00,06:53:00,9220186,1,,0,1,
3,06:55:00,06:55:00,9220222,2,,0,0,
3,06:58:00,06:58:00,9220196,3,,0,0,
3,07:00:00,07:00:00,9222132,4,,0,0,
3,07:02:00,07:02:00,9222131,5,,0,0,
3,07:05:00,07:05:00,9222151,6,,0,0,
3,07:07:00,07:07:00,9222280,7,,0,0,
3,07:09:00,07:09:00,9222085,8,,0,0,
3,07:11:00,07:11:00,9222088,9,,1,0,)";

TEST(loader_gtfs_route, read_stop_times_example_data) {
  auto stop_times =
      read_stop_times({STOP_TIMES_FILE, example_stop_times_file_content});

  for (auto const& trip : stop_times) {
    EXPECT_TRUE(trip.first == "AWD1" || trip.first == "AWE1");

    for (auto const& stop : trip.second) {
      if (trip.first == "AWE1") {
        switch (stop.first) {
          case 1:
            EXPECT_EQ("S1", stop.second.stop);
            EXPECT_EQ(6, stop.second.arr.time);
            EXPECT_EQ(6, stop.second.dep.time);
            EXPECT_TRUE(stop.second.arr.in_out_allowed);
            EXPECT_TRUE(stop.second.dep.in_out_allowed);
            break;
          case 2:
            EXPECT_EQ("S2", stop.second.stop);
            EXPECT_EQ(-1, stop.second.arr.time);
            EXPECT_EQ(-1, stop.second.dep.time);
            EXPECT_FALSE(stop.second.arr.in_out_allowed);
            EXPECT_TRUE(stop.second.dep.in_out_allowed);
            break;
          case 3:
            EXPECT_EQ("S3", stop.second.stop);
            EXPECT_EQ(6, stop.second.arr.time);
            EXPECT_EQ(6, stop.second.dep.time);
            EXPECT_TRUE(stop.second.arr.in_out_allowed);
            EXPECT_TRUE(stop.second.dep.in_out_allowed);
            break;
          case 4:
            EXPECT_EQ("S5", stop.second.stop);
            EXPECT_EQ(-1, stop.second.arr.time);
            EXPECT_EQ(-1, stop.second.dep.time);
            EXPECT_TRUE(stop.second.arr.in_out_allowed);
            EXPECT_TRUE(stop.second.dep.in_out_allowed);
            break;
          case 5:
            EXPECT_EQ("S6", stop.second.stop);
            EXPECT_EQ(6, stop.second.arr.time);
            EXPECT_EQ(6, stop.second.dep.time);
            EXPECT_TRUE(stop.second.arr.in_out_allowed);
            EXPECT_TRUE(stop.second.dep.in_out_allowed);
            break;
          default: EXPECT_FALSE(true);
        }
      } else if (trip.first == "AWD1") {
        switch (stop.first) {
          case 1:
            EXPECT_EQ("S1", stop.second.stop);
            EXPECT_EQ(6, stop.second.arr.time);
            EXPECT_EQ(6, stop.second.dep.time);
            EXPECT_TRUE(stop.second.arr.in_out_allowed);
            EXPECT_TRUE(stop.second.dep.in_out_allowed);
            break;
          case 2:
            EXPECT_EQ("S2", stop.second.stop);
            EXPECT_EQ(-1, stop.second.arr.time);
            EXPECT_EQ(-1, stop.second.dep.time);
            EXPECT_TRUE(stop.second.arr.in_out_allowed);
            EXPECT_TRUE(stop.second.dep.in_out_allowed);
            break;
          case 3:
            EXPECT_EQ("S3", stop.second.stop);
            EXPECT_EQ(6, stop.second.arr.time);
            EXPECT_EQ(6, stop.second.dep.time);
            EXPECT_TRUE(stop.second.arr.in_out_allowed);
            EXPECT_TRUE(stop.second.dep.in_out_allowed);
            break;
          case 4:
            EXPECT_EQ("S4", stop.second.stop);
            EXPECT_EQ(-1, stop.second.arr.time);
            EXPECT_EQ(-1, stop.second.dep.time);
            EXPECT_TRUE(stop.second.arr.in_out_allowed);
            EXPECT_TRUE(stop.second.dep.in_out_allowed);
            break;
          case 5:
            EXPECT_EQ("S5", stop.second.stop);
            EXPECT_EQ(-1, stop.second.arr.time);
            EXPECT_EQ(-1, stop.second.dep.time);
            EXPECT_TRUE(stop.second.arr.in_out_allowed);
            EXPECT_TRUE(stop.second.dep.in_out_allowed);
            break;
          case 6:
            EXPECT_EQ("S6", stop.second.stop);
            EXPECT_EQ(6, stop.second.arr.time);
            EXPECT_EQ(6, stop.second.dep.time);
            EXPECT_TRUE(stop.second.arr.in_out_allowed);
            EXPECT_TRUE(stop.second.dep.in_out_allowed);
            break;
          default: EXPECT_FALSE(true);
        }
      }
    }
  }
}

TEST(loader_gtfs_route, read_stop_times_berlin_data) {
  auto stop_times =
      read_stop_times({STOP_TIMES_FILE, berlin_stop_times_file_content});

  for (auto const& trip : stop_times) {
    EXPECT_TRUE(trip.first == "1" || trip.first == "2" || trip.first == "3");

    for (auto const& stop : trip.second) {
      if (trip.first == "1") {
        switch (stop.first) {
          case 1:
            EXPECT_EQ("9230999", stop.second.stop);
            EXPECT_EQ(4 * 60 + 45, stop.second.arr.time);
            EXPECT_EQ(4 * 60 + 46, stop.second.dep.time);
            EXPECT_FALSE(stop.second.arr.in_out_allowed);
            EXPECT_TRUE(stop.second.dep.in_out_allowed);
            break;
          case 4:
            EXPECT_EQ("9220070", stop.second.stop);
            EXPECT_EQ(5 * 60 + 6, stop.second.arr.time);
            EXPECT_EQ(5 * 60 + 6, stop.second.dep.time);
            EXPECT_FALSE(stop.second.arr.in_out_allowed);
            EXPECT_TRUE(stop.second.dep.in_out_allowed);
            break;
          case 7:
            EXPECT_EQ("9260024", stop.second.stop);
            EXPECT_EQ(5 * 60 + 32, stop.second.arr.time);
            EXPECT_EQ(5 * 60 + 32, stop.second.dep.time);
            EXPECT_TRUE(stop.second.arr.in_out_allowed);
            EXPECT_FALSE(stop.second.dep.in_out_allowed);
            break;
        }
      } else if (trip.first == "2") {
        switch (stop.first) {
          case 1:
            EXPECT_EQ("9260024", stop.second.stop);
            EXPECT_EQ(5 * 60 + 35, stop.second.arr.time);
            EXPECT_EQ(5 * 60 + 35, stop.second.dep.time);
            EXPECT_FALSE(stop.second.arr.in_out_allowed);
            EXPECT_TRUE(stop.second.dep.in_out_allowed);
            break;
          case 4:
            EXPECT_EQ("9220070", stop.second.stop);
            EXPECT_EQ(6 * 60 + 2, stop.second.arr.time);
            EXPECT_EQ(6 * 60 + 3, stop.second.dep.time);
            EXPECT_TRUE(stop.second.arr.in_out_allowed);
            EXPECT_FALSE(stop.second.dep.in_out_allowed);
            break;
          case 7:
            EXPECT_EQ("9230999", stop.second.stop);
            EXPECT_EQ(6 * 60 + 22, stop.second.arr.time);
            EXPECT_EQ(6 * 60 + 22, stop.second.dep.time);
            EXPECT_TRUE(stop.second.arr.in_out_allowed);
            EXPECT_FALSE(stop.second.dep.in_out_allowed);
            break;
        }
      } else if (trip.first == "3") {
        switch (stop.first) {
          case 9:
            EXPECT_EQ("9222088", stop.second.stop);
            EXPECT_EQ(7 * 60 + 11, stop.second.arr.time);
            EXPECT_EQ(7 * 60 + 11, stop.second.dep.time);
            EXPECT_TRUE(stop.second.arr.in_out_allowed);
            EXPECT_FALSE(stop.second.dep.in_out_allowed);
            break;
        }
      }
    }
  }
}

}  // gtfs
}  // loader
}  // motis
