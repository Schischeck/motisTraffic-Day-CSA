#include "gtest/gtest.h"

#include <algorithm>

#include "motis/loader/gtfs/trip.h"
#include "motis/loader/gtfs/files.h"

#include "./test_files.h"

using namespace parser;

namespace motis {
namespace loader {
namespace gtfs {

TEST(loader_gtfs_trip, read_trips_example_data) {
  auto agencies = read_agencies({AGENCY_FILE, example_agencies_file_content});
  auto routes =
      read_routes({ROUTES_FILE, example_routes_file_content}, agencies);
  auto dates = read_calendar_date(
      {CALENDAR_DATES_FILE, example_calendar_date_file_content});
  auto calendar = read_calendar({CALENDAR_FILE, example_calendar_file_content});
  auto services = traffic_days(calendar, dates);
  auto trips =
      read_trips({TRIPS_FILE, example_trips_file_content}, routes, services);

  EXPECT_EQ(2, trips.size());
  EXPECT_NE(end(trips), trips.find("AWE1"));
  EXPECT_EQ("A", trips["AWE1"]->route_->id_);
  EXPECT_EQ("Downtown", trips["AWE1"]->headsign_);
}

TEST(loader_gtfs_trip, read_trips_berlin_data) {
  auto agencies = read_agencies({AGENCY_FILE, berlin_agencies_file_content});
  auto routes =
      read_routes({ROUTES_FILE, berlin_routes_file_content}, agencies);
  auto dates = read_calendar_date(
      {CALENDAR_DATES_FILE, berlin_calendar_date_file_content});
  auto calendar = read_calendar({CALENDAR_FILE, berlin_calendar_file_content});
  auto services = traffic_days(calendar, dates);
  auto trips =
      read_trips({TRIPS_FILE, berlin_trips_file_content}, routes, services);

  EXPECT_EQ(3, trips.size());

  EXPECT_NE(end(trips), trips.find("1"));
  EXPECT_EQ("1", trips["1"]->route_->id_);
  // EXPECT_EQ("000856", trips["1"].service_id);
  EXPECT_EQ("Flughafen Schönefeld Terminal (Airport)", trips["1"]->headsign_);

  EXPECT_NE(end(trips), trips.find("2"));
  EXPECT_EQ("1", trips["2"]->route_->id_);
  // EXPECT_EQ("000856", trips["2"].service_id);
  EXPECT_EQ("S Potsdam Hauptbahnhof", trips["2"]->headsign_);

  EXPECT_NE(end(trips), trips.find("3"));
  EXPECT_EQ("2", trips["3"]->route_->id_);
  // EXPECT_EQ("000861", trips["3"].service_id);
  EXPECT_EQ("Golzow (PM), Schule", trips["3"]->headsign_);
}

}  // gtfs
}  // loader
}  // motis
