#include "gtest/gtest.h"

#include "motis/loader/parsers/gtfs/stop.h"
#include "motis/loader/parsers/gtfs/files.h"

#include "motis/schedule-format/Schedule_generated.h"

using namespace parser;

namespace motis {
namespace loader {
namespace gtfs {

cstr example_stops_file_content =
    R"(stop_id,stop_name,stop_desc,stop_lat,stop_lon,stop_url,location_type,parent_station
S1,Mission St. & Silver Ave.,The stop is located at the southwest corner of the intersection.,37.728631,-122.431282,,,
S2,Mission St. & Cortland Ave.,The stop is located 20 feet south of Mission St.,37.74103,-122.422482,,,
S3,Mission St. & 24th St.,The stop is located at the southwest corner of the intersection.,37.75223,-122.418581,,,
S4,Mission St. & 21st St.,The stop is located at the northwest corner of the intersection.,37.75713,-122.418982,,,
S5,Mission St. & 18th St.,The stop is located 25 feet west of 18th St.,37.761829,-122.419382,,,
S6,Mission St. & 15th St.,The stop is located 10 feet north of Mission St.,37.766629,-122.419782,,,
S7,24th St. Mission Station,,37.752240,-122.418450,,,S8
S8,24th St. Mission Station,,37.752240,-122.418450,http://www.bart.gov/stations/stationguide/stationoverview_24st.asp,1, )";

cstr berlin_stops_file_content =
    R"(stop_id,stop_code,stop_name,stop_desc,stop_lat,stop_lon,zone_id,stop_url,location_type,parent_station
5100071,,Zbaszynek,,52.2425040,15.8180870,,,0,
9230005,,S Potsdam Hauptbahnhof Nord,,52.3927320,13.0668480,,,0,
9230006,,"Potsdam, Charlottenhof Bhf",,52.3930040,13.0362980,,,0,)";

TEST(loader_gtfs_stop, read_stations_example_data) {
  flatbuffers::FlatBufferBuilder b;
  b.Finish(CreateSchedule(b, {},
                          b.CreateVector(read_stations(
                              {STOPS_FILE, example_stops_file_content}, b)),
                          {}, {}));

  auto schedule = GetSchedule(b.GetBufferPointer());
  auto stations = schedule->stations();

  ASSERT_TRUE(stations->size() == 8);

  auto station_1 = stations->Get(0);
  ASSERT_TRUE(station_1->id()->str() == "S1");
  ASSERT_TRUE(station_1->name()->str() == "Mission St. & Silver Ave.");
  ASSERT_TRUE(std::abs(station_1->lat() - 37.728631) < 0.00001);
  ASSERT_TRUE(std::abs(station_1->lng() - -122.431282) < 0.00001);

  auto station_6 = stations->Get(5);
  ASSERT_TRUE(station_6->id()->str() == "S6");
  ASSERT_TRUE(station_6->name()->str() == "Mission St. & 15th St.");
  ASSERT_TRUE(std::abs(station_6->lat() - 37.766629) < 0.00001);
  ASSERT_TRUE(std::abs(station_6->lng() - -122.419782) < 0.00001);

  auto station_8 = stations->Get(7);
  ASSERT_TRUE(station_8->id()->str() == "S8");
  ASSERT_TRUE(station_8->name()->str() == "24th St. Mission Station");
  ASSERT_TRUE(std::abs(station_8->lat() - 37.752240) < 0.00001);
  ASSERT_TRUE(std::abs(station_8->lng() - -122.418450) < 0.00001);
}

TEST(loader_gtfs_stop, read_stations_berlin_data) {
  flatbuffers::FlatBufferBuilder b;
  b.Finish(CreateSchedule(
      b, {},
      b.CreateVector(read_stations({STOPS_FILE, berlin_stops_file_content}, b)),
      {}, {}));

  auto schedule = GetSchedule(b.GetBufferPointer());
  auto stations = schedule->stations();

  ASSERT_TRUE(stations->size() == 3);

  auto station_1 = stations->Get(0);
  ASSERT_TRUE(station_1->id()->str() == "5100071");
  ASSERT_TRUE(station_1->name()->str() == "Zbaszynek");
  ASSERT_TRUE(std::abs(station_1->lat() - 52.2425040) < 0.00001);
  ASSERT_TRUE(std::abs(station_1->lng() - 15.8180870) < 0.00001);

  auto station_6 = stations->Get(1);
  ASSERT_TRUE(station_6->id()->str() == "9230005");
  ASSERT_TRUE(station_6->name()->str() == "S Potsdam Hauptbahnhof Nord");
  ASSERT_TRUE(std::abs(station_6->lat() - 52.3927320) < 0.00001);
  ASSERT_TRUE(std::abs(station_6->lng() - 13.0668480) < 0.00001);

  auto station_8 = stations->Get(2);
  ASSERT_TRUE(station_8->id()->str() == "9230006");
  ASSERT_TRUE(station_8->name()->str() == "Potsdam, Charlottenhof Bhf");
  ASSERT_TRUE(std::abs(station_8->lat() - 52.3930040) < 0.00001);
  ASSERT_TRUE(std::abs(station_8->lng() - 13.0362980) < 0.00001);
}

}  // gtfs
}  // loader
}  // motis
