#include <cstring>

#include "boost/filesystem/path.hpp"

#include "catch/catch.hpp"

#include "parser/file.h"

#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/parsers/hrd/files.h"
#include "motis/loader/parsers/hrd/stations_parser.h"
#include "motis/schedule-format/Schedule_generated.h"

using namespace parser;
namespace fs = boost::filesystem;

namespace motis {
namespace loader {
namespace hrd {

fs::path res_path("motis/loader/test_resources/");

TEST_CASE("parse_stations") {
  auto stations_file_buf =
      file((res_path / STATIONS_FILE).c_str(), "ro").content();
  cstr stations_file_content(static_cast<char const*>(stations_file_buf.buf_),
                             stations_file_buf.size_);

  auto coordinates_file_buf =
      file((res_path / COORDINATES_FILE).c_str(), "ro").content();
  cstr coordinates_file_content(
      static_cast<char const*>(coordinates_file_buf.buf_),
      coordinates_file_buf.size_);

  flatbuffers::FlatBufferBuilder b;
  auto station_data =
      parse_stations({STATIONS_FILE, stations_file_content},
                     {COORDINATES_FILE, coordinates_file_content}, b);

  b.Finish(CreateSchedule(b, {}, b.CreateVector(values(station_data)), {}, {}));

  auto schedule = GetSchedule(b.GetBufferPointer());
  auto stations = schedule->stations();

  REQUIRE(stations->size() == 2);

  REQUIRE(stations->Get(0)->id()->str() == "100001");
  REQUIRE(stations->Get(0)->name()->str() == "Hauptwache, Frankfurt am Main");
  REQUIRE(std::abs(stations->Get(0)->lng() - 8.679296) <= 0.001);
  REQUIRE(std::abs(stations->Get(0)->lat() - 50.113963) <= 0.001);

  REQUIRE(stations->Get(1)->id()->str() == "100002");
  REQUIRE(stations->Get(1)->name()->str() ==
          "Römer/Paulskirche, Frankfurt am Main");
  REQUIRE(std::abs(stations->Get(1)->lng() - 8.681793) <= 0.001);
  REQUIRE(std::abs(stations->Get(1)->lat() - 50.110902) <= 0.001);

  REQUIRE(station_data.size() == 2);
  REQUIRE(station_data.find(100001) != end(station_data));
  REQUIRE(station_data.find(100002) != end(station_data));
}

}  // hrd
}  // loader
}  // motis
