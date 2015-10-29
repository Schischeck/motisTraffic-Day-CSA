#include <cstring>

#include "boost/filesystem/path.hpp"

#include "gtest/gtest.h"

#include "parser/file.h"

#include "test_spec.h"
#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/parsers/hrd/files.h"
#include "motis/loader/parsers/hrd/stations_parser.h"
#include "motis/schedule-format/Schedule_generated.h"
#include "motis/loader/parsers/hrd/station_meta_data_parser.h"

using namespace parser;
namespace fs = boost::filesystem;

namespace motis {
namespace loader {
namespace hrd {

TEST(loader_hrd_stations, parse_stations) {

  auto stations_file_buf = load_file(TEST_RESOURCES / STATIONS_FILE);
  cstr stations_file_content(
      {stations_file_buf.data(), stations_file_buf.size()});

  auto coordinates_file_buf =
      file((TEST_RESOURCES / COORDINATES_FILE).c_str(), "ro").content();
  cstr coordinates_file_content(
      static_cast<char const*>(coordinates_file_buf.buf_),
      coordinates_file_buf.size_);

  auto infotext_file_buf = load_file(TEST_RESOURCES / "infotext_minimal.101");
  cstr infotext_file_content(static_cast<char const*>(infotext_file_buf.buf_),
                             infotext_file_buf.size_);

  station_meta_data metas;
  parse_station_meta_data({INFOTEXT_FILE, infotext_file_content}, metas);

  auto stations =
      parse_stations({STATIONS_FILE, stations_file_content},
                     {COORDINATES_FILE, coordinates_file_content}, metas);

  ASSERT_EQ(2, stations.size());

  auto it = stations.find(100001);
  ASSERT_TRUE(it != end(stations));

  auto station = it->second;
  ASSERT_STREQ("Hauptwache, Frankfurt am Main", station.name.c_str());
  ASSERT_TRUE(std::abs(station.lng - 8.679296) <= 0.001);
  ASSERT_TRUE(std::abs(station.lat - 50.113963) <= 0.001);
  ASSERT_EQ(5, station.change_time);

  it = stations.find(100002);
  ASSERT_TRUE(it != end(stations));

  station = it->second;
  ASSERT_STREQ("Roemer/Paulskirche, Frankfurt am Main", station.name.c_str());
  ASSERT_TRUE(std::abs(station.lng - 8.681793) <= 0.001);
  ASSERT_TRUE(std::abs(station.lat - 50.110902) <= 0.001);
  ASSERT_EQ(5, station.change_time);
}

}  // hrd
}  // loader
}  // motis
