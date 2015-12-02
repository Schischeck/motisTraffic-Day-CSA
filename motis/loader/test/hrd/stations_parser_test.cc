#include <cstring>

#include "gtest/gtest.h"

#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/hrd/parser/station_meta_data_parser.h"
#include "motis/loader/hrd/parser/stations_parser.h"

namespace motis {
namespace loader {
namespace hrd {

constexpr char const* infotext = R"(%
0000001 ICE International
0000002 Canopus
0000003 Metropol
0000004 Kopernikus
0088506 2002-05-01:N:Darmstadt Hbf:8000068:FD:68:008011336:DE:06411000:Darmstadt Hbf:DA
0599553 140852529:2015-08-18T00:01:00Z
0088540 2002-05-01:N:Frankfurt(Main)Hbf:8000105:FF:105:008011068:DE:06412000:Frankfurt(Main)Hbf
0599554 140852530:2015-08-18T00:01:00Z
0599555 140852531:2015-08-18T00:01:00Z
0599556 140852532:2015-08-18T00:01:00Z
)";

constexpr char const* footpaths_old = R"(%
8003919 0721747 015
8003935 0651301 003
)";

constexpr char const* footpaths_new = R"(%
8003918 0721747 015
*A XQ
*U 1
8003935 0651301 005

0663370: F0651591  0663370
)";

constexpr char const* stations_data = R"(%
0100001     Hauptwache, Frankfurt am Main
0100002     Roemer/Paulskirche, Frankfurt am Main
)";

constexpr char const* coordinates_data = R"(%
0100001   8.679296  50.113963 Hauptwache, Frankfurt am Main
0100002   8.681793  50.110902 Roemer/Paulskirche, Frankfurt am Main
)";

TEST(loader_hrd_stations_parser, meta_data) {
  try {
    station_meta_data metas;
    loaded_file info_text_file("infotext.101", infotext);
    loaded_file fp_old_file("footpaths_old.101", footpaths_old);
    loaded_file fp_new_file("footpaths_new.101", footpaths_new);
    parse_station_meta_data(info_text_file, fp_old_file, fp_new_file, metas);

    ASSERT_EQ(2, metas.station_change_times_.size());
    ASSERT_EQ(7, metas.get_station_change_time(8000068));
    ASSERT_EQ(8, metas.get_station_change_time(8000105));

    ASSERT_EQ(3, metas.footpaths_.find({8003935, 651301, -1})->duration);
  } catch (parser_error const& pe) {
    pe.print_what();
    ASSERT_TRUE(false);
  }
}

TEST(loader_hrd_stations_parser, parse_stations) {

  try {
    station_meta_data metas;
    parse_station_meta_data({"infotext.101", infotext},
                            {"metabhf.101", footpaths_old},
                            {"metabhf_zusatz.101", footpaths_new}, metas);
    auto stations = parse_stations({"bahnhof.101", stations_data},
                                   {"dbkoords.101", coordinates_data}, metas);

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
  } catch (parser_error const& pe) {
    pe.print_what();
    ASSERT_TRUE(false);
  }
}

}  // hrd
}  // loader
}  // motis
