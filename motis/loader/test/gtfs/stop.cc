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
                          {}));

  auto schedule = GetSchedule(b.GetBufferPointer());
  auto stations = schedule->stations();

  ASSERT_TRUE(stations->size() == 8);

  auto station_1 = stations->Get(0);
  ASSERT_TRUE(station_1->id()->str() == "S1");
  ASSERT_TRUE(station_1->name()->str() == "Mission St. & Silver Ave.");
  ASSERT_FLOAT_EQ(37.728631, station_1->lat());
  ASSERT_FLOAT_EQ(-122.431282, station_1->lng());

  auto station_6 = stations->Get(5);
  ASSERT_TRUE(station_6->id()->str() == "S6");
  ASSERT_TRUE(station_6->name()->str() == "Mission St. & 15th St.");
  ASSERT_FLOAT_EQ(37.766629, station_6->lat());
  ASSERT_FLOAT_EQ(-122.419782, station_6->lng());

  auto station_8 = stations->Get(7);
  ASSERT_TRUE(station_8->id()->str() == "S8");
  ASSERT_TRUE(station_8->name()->str() == "24th St. Mission Station");
  ASSERT_FLOAT_EQ(37.752240, station_8->lat());
  ASSERT_FLOAT_EQ(-122.418450, station_8->lng());
}

TEST(loader_gtfs_stop, read_stations_berlin_data) {
  flatbuffers::FlatBufferBuilder b;
  b.Finish(CreateSchedule(
      b, {},
      b.CreateVector(read_stations({STOPS_FILE, berlin_stops_file_content}, b)),
      {}));

  auto schedule = GetSchedule(b.GetBufferPointer());
  auto stations = schedule->stations();

  ASSERT_TRUE(stations->size() == 3);

  auto station_1 = stations->Get(0);
  ASSERT_TRUE(station_1->id()->str() == "5100071");
  ASSERT_TRUE(station_1->name()->str() == "Zbaszynek");
  ASSERT_FLOAT_EQ(52.2425040, station_1->lat());
  ASSERT_FLOAT_EQ(15.8180870, station_1->lng());

  auto station_6 = stations->Get(1);
  ASSERT_TRUE(station_6->id()->str() == "9230005");
  ASSERT_TRUE(station_6->name()->str() == "S Potsdam Hauptbahnhof Nord");
  ASSERT_FLOAT_EQ(52.3927320, station_6->lat());
  ASSERT_FLOAT_EQ(13.0668480, station_6->lng());

  auto station_8 = stations->Get(2);
  ASSERT_TRUE(station_8->id()->str() == "9230006");
  ASSERT_TRUE(station_8->name()->str() == "Potsdam, Charlottenhof Bhf");
  ASSERT_FLOAT_EQ(52.3930040, station_8->lat());
  ASSERT_FLOAT_EQ(13.0362980, station_8->lng());
}

}  // gtfs
}  // loader
}  // motis
