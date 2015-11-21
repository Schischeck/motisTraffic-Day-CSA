#include "motis/loader/gtfs/route.h"

#include <tuple>
#include <algorithm>

#include "parser/csv.h"

#include "motis/loader/util.h"

using namespace parser;
using std::get;

namespace motis {
namespace loader {
namespace gtfs {

std::map<unsigned, std::string> route::s_types_ = {
    // clang-format off
  { 0, "Tram, Light Rail, Streetcar"},
  { 1, "Subway, Metro"},
  { 2, "Rail"},
  { 3, "Bus"},
  { 4, "Ferry"},
  { 5, "Cable Car"},
  { 6, "Gondola, Suspended cable car"},
  { 7, "Funicular"},
  { 100, "Railway Service" },
  { 101, "High Speed Rail Service" },
  { 102, "Long Distance Trains" },
  { 103, "Inter Regional Rail Service" },
  { 104, "Car Transport Rail Service" },
  { 105, "Sleeper Rail Service" },
  { 106, "Regional Rail Service" },
  { 107, "Tourist Railway Service" },
  { 108, "Rail Shuttle (Within Complex)" },
  { 109, "Suburban Railway" },
  { 110, "Replacement Rail Service" },
  { 111, "Special Rail Service" },
  { 112, "Lorry Transport Rail Service" },
  { 113, "All Rail Services" },
  { 114, "Cross-Country Rail Service" },
  { 115, "Vehicle Transport Rail Service" },
  { 116, "Rack and Pinion Railway" },
  { 117, "Additional Rail Service" },
  { 200, "Coach Service" },
  { 201, "International Coach Service" },
  { 202, "National Coach Service" },
  { 203, "Shuttle Coach Service" },
  { 204, "Regional Coach Service" },
  { 205, "Special Coach Service" },
  { 206, "Sightseeing Coach Service" },
  { 207, "Tourist Coach Service" },
  { 208, "Commuter Coach Service" },
  { 209, "All Coach Services" },
  { 300, "Suburban Railway Service" },
  { 400, "Urban Railway Service" },
  { 401, "Metro Service" },
  { 402, "Underground Service" },
  { 403, "Urban Railway Service" },
  { 404, "All Urban Railway Services" },
  { 500, "Metro Service" },
  { 600, "Underground Service" },
  { 700, "Bus Service" },
  { 701, "Regional Bus Service" },
  { 702, "Express Bus Service" },
  { 703, "Stopping Bus Service" },
  { 704, "Local Bus Service" },
  { 705, "Night Bus Service" },
  { 706, "Post Bus Service" },
  { 707, "Special Needs Bus" },
  { 708, "Mobility Bus Service" },
  { 709, "Mobility Bus for Registered Disabled" },
  { 710, "Sightseeing Bus" },
  { 711, "Shuttle Bus" },
  { 712, "School Bus" },
  { 713, "School and Public Service Bus" },
  { 714, "Rail Replacement Bus Service" },
  { 715, "Demand and Response Bus Service" },
  { 716, "All Bus Services" },
  { 800, "Trolleybus Service" },
  { 900, "Tram Service" },
  { 901, "City Tram Service" },
  { 902, "Local Tram Service" },
  { 903, "Regional Tram Service" },
  { 904, "Sightseeing Tram Service" },
  { 905, "Shuttle Tram Service" },
  { 906, "All Tram Services" },
  { 1000, "Water Transport Service" },
  { 1001, "International Car Ferry Service" },
  { 1002, "National Car Ferry Service" },
  { 1003, "Regional Car Ferry Service" },
  { 1004, "Local Car Ferry Service" },
  { 1005, "International Passenger Ferry Service" },
  { 1006, "National Passenger Ferry Service" },
  { 1007, "Regional Passenger Ferry Service" },
  { 1008, "Local Passenger Ferry Service" },
  { 1009, "Post Boat Service" },
  { 1010, "Train Ferry Service" },
  { 1011, "Road-Link Ferry Service" },
  { 1012, "Airport-Link Ferry Service" },
  { 1013, "Car High-Speed Ferry Service" },
  { 1014, "Passenger High-Speed Ferry Service" },
  { 1015, "Sightseeing Boat Service" },
  { 1016, "School Boat" },
  { 1017, "Cable-Drawn Boat Service" },
  { 1018, "River Bus Service" },
  { 1019, "Scheduled Ferry Service" },
  { 1020, "Shuttle Ferry Service" },
  { 1021, "All Water Transport Services" },
  { 1100, "Air Service" },
  { 1101, "International Air Service" },
  { 1102, "Domestic Air Service" },
  { 1103, "Intercontinental Air Service" },
  { 1104, "Domestic Scheduled Air Service" },
  { 1105, "Shuttle Air Service" },
  { 1106, "Intercontinental Charter Air Service" },
  { 1107, "International Charter Air Service" },
  { 1108, "Round-Trip Charter Air Service" },
  { 1109, "Sightseeing Air Service" },
  { 1110, "Helicopter Air Service" },
  { 1111, "Domestic Charter Air Service" },
  { 1112, "Schengen-Area Air Service" },
  { 1113, "Airship Service" },
  { 1114, "All Air Services" },
  { 1200, "Ferry Service" },
  { 1300, "Telecabin Service" },
  { 1301, "Telecabin Service" },
  { 1302, "Cable Car Service" },
  { 1303, "Elevator Service" },
  { 1304, "Chair Lift Service" },
  { 1305, "Drag Lift Service" },
  { 1306, "Small Telecabin Service" },
  { 1307, "All Telecabin Services" },
  { 1400, "Funicular Service" },
  { 1401, "Funicular Service" },
  { 1402, "All Funicular Service" },
  { 1500, "Taxi Service" },
  { 1501, "Communal Taxi Service" },
  { 1502, "Water Taxi Service" },
  { 1503, "Rail Taxi Service" },
  { 1504, "Bike Taxi Service" },
  { 1505, "Licensed Taxi Service" },
  { 1506, "Private Hire Service Vehicle" },
  { 1507, "All Taxi Services" },
  { 1600, "Self Drive" },
  { 1601, "Hire Car" },
  { 1602, "Hire Van" },
  { 1603, "Hire Motorbike" },
  { 1604, "Hire Cycle" },
  { 1605, "All Self-Drive Vehicles" }
    // clang-format on
};

using gtfs_route = std::tuple<cstr, cstr, cstr, cstr, int>;
enum { route_id, agency_id, route_short_name, route_long_name, route_type };
static const column_mapping<gtfs_route> columns = {
    {"route_id", "agency_id", "route_short_name", "route_long_name",
     "route_type"}};

route_map read_routes(loaded_file file, agency_map const& agencies) {
  route_map routes;
  for (auto const& r : read<gtfs_route>(file.content(), columns)) {
    auto agency_it = agencies.find(get<agency_id>(r).to_str());
    auto agency_ptr =
        agency_it == end(agencies) ? nullptr : agency_it->second.get();
    routes.emplace(get<route_id>(r).to_str(),
                   make_unique<route>(agency_ptr, get<route_id>(r).to_str(),
                                      get<route_short_name>(r).to_str(),
                                      get<route_long_name>(r).to_str(),
                                      get<route_type>(r)));
  }
  return routes;
}

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
