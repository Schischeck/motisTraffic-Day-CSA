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

  flatbuffers::FlatBufferBuilder b;
  auto station_data =
      parse_stations({STATIONS_FILE, stations_file_content},
                     {COORDINATES_FILE, coordinates_file_content}, metas, b);

  b.Finish(CreateSchedule(b, {}, b.CreateVector(values(station_data)), {}));

  auto schedule = GetSchedule(b.GetBufferPointer());
  auto stations = schedule->stations();

  ASSERT_TRUE(stations->size() == 2);

  ASSERT_TRUE(stations->Get(0)->id()->str() == "100001");
  ASSERT_TRUE(stations->Get(0)->name()->str() ==
              "Hauptwache, Frankfurt am Main");
  ASSERT_TRUE(std::abs(stations->Get(0)->lng() - 8.679296) <= 0.001);
  ASSERT_TRUE(std::abs(stations->Get(0)->lat() - 50.113963) <= 0.001);
  ASSERT_EQ(5, stations->Get(0)->interchange_time());

  ASSERT_TRUE(stations->Get(1)->id()->str() == "100002");
  ASSERT_TRUE(stations->Get(1)->name()->str() ==
              "Römer/Paulskirche, Frankfurt am Main");
  ASSERT_TRUE(std::abs(stations->Get(1)->lng() - 8.681793) <= 0.001);
  ASSERT_TRUE(std::abs(stations->Get(1)->lat() - 50.110902) <= 0.001);
  ASSERT_EQ(5, stations->Get(1)->interchange_time());

  ASSERT_TRUE(station_data.size() == 2);
  ASSERT_TRUE(station_data.find(100001) != end(station_data));
  ASSERT_TRUE(station_data.find(100002) != end(station_data));
}

}  // hrd
}  // loader
}  // motis
