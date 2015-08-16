#include <cstring>

#include "catch/catch.hpp"

#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/parsers/hrd/files.h"
#include "motis/loader/parsers/hrd/stations_parser.h"
#include "motis/schedule-format/Schedule_generated.h"

using namespace parser;

namespace motis {
namespace loader {
namespace hrd {

TEST_CASE("parse_stations") {
  cstr stations_file_content =
      "0100001     Hauptwache, Frankfurt am Main\n"
      "0100002     Römer/Paulskirche, Frankfurt am Main";

  cstr coordinates_file_content =
      "0100001 8.67929600 50.1139630        % Hauptwache, Frankfurt am Main\n"
      "0100002 8.68179300 50.1109020        ";

  flatbuffers::FlatBufferBuilder b;
  auto station_data =
      parse_stations({STATIONS_FILE, stations_file_content},
                     {COORDINATES_FILE, coordinates_file_content}, b);

  b.Finish(
      CreateSchedule(b, {}, b.CreateVector(values(station_data)), {}, {}, {}));

  auto schedule = GetSchedule(b.GetBufferPointer());
  auto stations = schedule->stations();

  REQUIRE(stations->size() == 2);

  REQUIRE(stations->Get(0)->id()->str() == "100001");
  REQUIRE(stations->Get(0)->name()->str() == "Hauptwache, Frankfurt am Main");
  REQUIRE(std::abs(stations->Get(0)->lng() - 8.679296) <= 0.001);
  REQUIRE(std::abs(stations->Get(0)->lat() - 50.1139630) <= 0.001);

  REQUIRE(stations->Get(1)->id()->str() == "100002");
  REQUIRE(stations->Get(1)->name()->str() ==
          "Römer/Paulskirche, Frankfurt am Main");
  REQUIRE(std::abs(stations->Get(1)->lng() - 8.681793) <= 0.001);
  REQUIRE(std::abs(stations->Get(1)->lat() - 50.1109020) <= 0.001);

  REQUIRE(station_data.size() == 2);
  REQUIRE(station_data.find(100001) != end(station_data));
  REQUIRE(station_data.find(100002) != end(station_data));
}

}  // hrd
}  // loader
}  // motis
