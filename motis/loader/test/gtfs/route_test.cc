#include "gtest/gtest.h"

#include "motis/loader/parsers/gtfs/route.h"
#include "motis/loader/parsers/gtfs/files.h"

#include "motis/schedule-format/Schedule_generated.h"

using namespace parser;

namespace motis {
namespace loader {
namespace gtfs {

cstr example_routes_file_content =
    R"(route_id,route_short_name,route_long_name,route_desc,route_type
A,17,Mission,"The ""A"" route travels from lower Mission to Downtown.",3)";

cstr berlin_routes_file_content =
    R"(route_id,agency_id,route_short_name,route_long_name,route_desc,route_type,route_url,route_color,route_text_color
1,ANG---,SXF2,,,700,http://www.vbb.de,,
10,BMO---,927,,,700,http://www.vbb.de,,
809,N04---,,"Leisnig -- Leipzig, Hauptbahnhof",,100,http://www.vbb.de,,
81,BON---,2/412,,,700,http://www.vbb.de,,
810,N04---,,"S+U Lichtenberg Bhf (Berlin) -- Senftenberg, Bahnhof",,100,http://www.vbb.de,,
811,N04---,,"S+U Lichtenberg Bhf (Berlin) -- Altdöbern, Bahnhof",,100,http://www.vbb.de,,
812,N04---,RB14,,,100,http://www.vbb.de,,)";

TEST(loader_gtfs_route, read_routes_example_data) {
  auto routes = read_routes({ROUTES_FILE, example_routes_file_content});

  EXPECT_EQ(1, routes.size());
  EXPECT_NE(end(routes), routes.find("A"));
  EXPECT_EQ("", routes["A"].agency_id);
  EXPECT_EQ("17", routes["A"].short_name);
  EXPECT_EQ("Mission", routes["A"].long_name);
  EXPECT_EQ(3, routes["A"].type);
}


TEST(loader_gtfs_route, read_routes_berlin_data) {
  auto routes = read_routes({ROUTES_FILE, berlin_routes_file_content});

  EXPECT_EQ(7, routes.size());

  ASSERT_NE(end(routes), routes.find("1"));
  EXPECT_EQ("ANG---", routes["1"].agency_id);
  EXPECT_EQ("SXF2", routes["1"].short_name);
  EXPECT_EQ("", routes["1"].long_name);
  EXPECT_EQ(700, routes["1"].type);

  ASSERT_NE(end(routes), routes.find("809"));
  EXPECT_EQ("N04---", routes["809"].agency_id);
  EXPECT_EQ("", routes["809"].short_name);
  EXPECT_EQ("Leisnig -- Leipzig, Hauptbahnhof", routes["809"].long_name);
  EXPECT_EQ(100, routes["809"].type);

  ASSERT_NE(end(routes), routes.find("812"));
  EXPECT_EQ("N04---", routes["812"].agency_id);
  EXPECT_EQ("RB14", routes["812"].short_name);
  EXPECT_EQ("", routes["812"].long_name);
  EXPECT_EQ(100, routes["812"].type);
}

}  // gtfs
}  // loader
}  // motis
