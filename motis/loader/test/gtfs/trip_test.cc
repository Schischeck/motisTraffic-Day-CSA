#include "gtest/gtest.h"

#include "motis/loader/gtfs/trip.h"
#include "motis/loader/gtfs/files.h"

using namespace parser;

namespace motis {
namespace loader {
namespace gtfs {

const char* example_trips_file_content =
    R"(route_id,service_id,trip_id,trip_headsign,block_id
A,WE,AWE1,Downtown,1
A,WE,AWE2,Downtown,2)";

const char* berlin_trips_file_content =
    R"(route_id,service_id,trip_id,trip_headsign,trip_short_name,direction_id,block_id,shape_id
1,000856,1,Flughafen Schönefeld Terminal (Airport),,,1,
1,000856,2,S Potsdam Hauptbahnhof,,,2,
2,000861,3,"Golzow (PM), Schule",,,3,)";

TEST(loader_gtfs_trip, read_trips_example_data) {
  auto trips = read_trips({TRIPS_FILE, example_trips_file_content});

  EXPECT_EQ(2, trips.size());
  EXPECT_NE(end(trips), trips.find("AWE1"));
  EXPECT_EQ("A", trips["AWE1"].route_id);
  EXPECT_EQ("WE", trips["AWE1"].service_id);
  EXPECT_EQ("Downtown", trips["AWE1"].headsign);
}

TEST(loader_gtfs_trip, read_trips_berlin_data) {
  auto trips = read_trips({TRIPS_FILE, berlin_trips_file_content});

  EXPECT_EQ(3, trips.size());

  EXPECT_NE(end(trips), trips.find("1"));
  EXPECT_EQ("1", trips["1"].route_id);
  EXPECT_EQ("000856", trips["1"].service_id);
  EXPECT_EQ("Flughafen Schönefeld Terminal (Airport)", trips["1"].headsign);

  EXPECT_NE(end(trips), trips.find("2"));
  EXPECT_EQ("1", trips["2"].route_id);
  EXPECT_EQ("000856", trips["2"].service_id);
  EXPECT_EQ("S Potsdam Hauptbahnhof", trips["2"].headsign);

  EXPECT_NE(end(trips), trips.find("3"));
  EXPECT_EQ("2", trips["3"].route_id);
  EXPECT_EQ("000861", trips["3"].service_id);
  EXPECT_EQ("Golzow (PM), Schule", trips["3"].headsign);
}

}  // gtfs
}  // loader
}  // motis
