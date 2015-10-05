#include "gtest/gtest.h"

#include "motis/loader/parsers/gtfs/stop.h"
#include "motis/loader/parsers/gtfs/files.h"
#include "motis/loader/util.h"
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
  auto station_map = read_stations({STOPS_FILE, example_stops_file_content}, b);

  b.Finish(CreateSchedule(b, {}, b.CreateVector(values(station_map))));

  auto schedule = GetSchedule(b.GetBufferPointer());
  auto stations = schedule->stations();

  ASSERT_TRUE(stations->size() == 8);

  for (auto const& s : *stations) {
    if (s->id()->str() == "S1") {
      ASSERT_STREQ("Mission St. & Silver Ave.", s->name()->c_str());
      ASSERT_FLOAT_EQ(37.728631, s->lat());
      ASSERT_FLOAT_EQ(-122.431282, s->lng());
    } else if (s->id()->str() == "S6") {
      ASSERT_TRUE(s->name()->str() == "Mission St. & 15th St.");
      ASSERT_FLOAT_EQ(37.766629, s->lat());
      ASSERT_FLOAT_EQ(-122.419782, s->lng());
    } else if (s->id()->str() == "S8") {
      ASSERT_TRUE(s->name()->str() == "24th St. Mission Station");
      ASSERT_FLOAT_EQ(37.752240, s->lat());
      ASSERT_FLOAT_EQ(-122.418450, s->lng());
    }
  }
}

TEST(loader_gtfs_stop, read_stations_berlin_data) {
  flatbuffers::FlatBufferBuilder b;
  b.Finish(CreateSchedule(b, {},
                          b.CreateVector(values(read_stations(
                              {STOPS_FILE, berlin_stops_file_content}, b))),
                          {}));

  auto schedule = GetSchedule(b.GetBufferPointer());
  auto stations = schedule->stations();

  ASSERT_TRUE(stations->size() == 3);

  for (auto const& s : *stations) {
    if (s->id()->str() == "5100071") {
      ASSERT_TRUE(s->name()->str() == "Zbaszynek");
      ASSERT_FLOAT_EQ(52.2425040, s->lat());
      ASSERT_FLOAT_EQ(15.8180870, s->lng());
    } else if (s->id()->str() == "9230005") {
      ASSERT_TRUE(s->name()->str() == "S Potsdam Hauptbahnhof Nord");
      ASSERT_FLOAT_EQ(52.3927320, s->lat());
      ASSERT_FLOAT_EQ(13.0668480, s->lng());
    } else if (s->id()->str() == "9230006") {
      ASSERT_TRUE(s->name()->str() == "Potsdam, Charlottenhof Bhf");
      ASSERT_FLOAT_EQ(52.3930040, s->lat());
      ASSERT_FLOAT_EQ(13.0362980, s->lng());
    }
  }
}

}  // gtfs
}  // loader
}  // motis
