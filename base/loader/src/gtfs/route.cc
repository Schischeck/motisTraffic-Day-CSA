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

// Source: https://groups.google.com/d/msg/gtfs-changes/keT5rTPS7Y0/71uMz2l6ke0J
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
  { 100, "Zug" },
  { 101, "High Speed Rail" },
  { 102, "Long Distance Trains" },
  { 103, "Inter Regional Rail" },
  { 104, "Car Transport Rail" },
  { 105, "Sleeper Rail" },
  { 106, "Regional Rail" },
  { 107, "Tourist Railway" },
  { 108, "Rail Shuttle (Within Complex)" },
  { 109, "S" },
  { 110, "Replacement Rail" },
  { 111, "Special Rail" },
  { 112, "Lorry Transport Rail" },
  { 113, "All Rails" },
  { 114, "Cross-Country Rail" },
  { 115, "Vehicle Transport Rail" },
  { 116, "Rack and Pinion Railway" },
  { 117, "Additional Rail" },
  { 200, "Coach" },
  { 201, "International Coach" },
  { 202, "National Coach" },
  { 203, "Shuttle Coach" },
  { 204, "Regional Coach" },
  { 205, "Special Coach" },
  { 206, "Sightseeing Coach" },
  { 207, "Tourist Coach" },
  { 208, "Commuter Coach" },
  { 209, "All Coachs" },
  { 300, "Suburban Railway" },
  { 400, "Urban Railway" },
  { 401, "Metro" },
  { 402, "Underground" },
  { 403, "Urban Railway" },
  { 404, "All Urban Railways" },
  { 500, "Metro" },
  { 600, "Underground" },
  { 700, "Bus" },
  { 701, "Regional Bus" },
  { 702, "Express Bus" },
  { 703, "Stopping Bus" },
  { 704, "Local Bus" },
  { 705, "Night Bus" },
  { 706, "Post Bus" },
  { 707, "Special Needs Bus" },
  { 708, "Mobility Bus" },
  { 709, "Mobility Bus for Registered Disabled" },
  { 710, "Sightseeing Bus" },
  { 711, "Shuttle Bus" },
  { 712, "School Bus" },
  { 713, "School and Public Bus" },
  { 714, "Rail Replacement Bus" },
  { 715, "Demand and Response Bus" },
  { 716, "All Buss" },
  { 800, "Trolleybus" },
  { 900, "Str" },
  { 901, "City Tram" },
  { 902, "Local Tram" },
  { 903, "Regional Tram" },
  { 904, "Sightseeing Tram" },
  { 905, "Shuttle Tram" },
  { 906, "All Trams" },
  { 1000, "Water Transport" },
  { 1001, "International Car Ferry" },
  { 1002, "National Car Ferry" },
  { 1003, "Regional Car Ferry" },
  { 1004, "Local Car Ferry" },
  { 1005, "International Passenger Ferry" },
  { 1006, "National Passenger Ferry" },
  { 1007, "Regional Passenger Ferry" },
  { 1008, "Local Passenger Ferry" },
  { 1009, "Post Boat" },
  { 1010, "Train Ferry" },
  { 1011, "Road-Link Ferry" },
  { 1012, "Airport-Link Ferry" },
  { 1013, "Car High-Speed Ferry" },
  { 1014, "Passenger High-Speed Ferry" },
  { 1015, "Sightseeing Boat" },
  { 1016, "School Boat" },
  { 1017, "Cable-Drawn Boat" },
  { 1018, "River Bus" },
  { 1019, "Scheduled Ferry" },
  { 1020, "Shuttle Ferry" },
  { 1021, "All Water Transports" },
  { 1100, "Air" },
  { 1101, "International Air" },
  { 1102, "Domestic Air" },
  { 1103, "Intercontinental Air" },
  { 1104, "Domestic Scheduled Air" },
  { 1105, "Shuttle Air" },
  { 1106, "Intercontinental Charter Air" },
  { 1107, "International Charter Air" },
  { 1108, "Round-Trip Charter Air" },
  { 1109, "Sightseeing Air" },
  { 1110, "Helicopter Air" },
  { 1111, "Domestic Charter Air" },
  { 1112, "Schengen-Area Air" },
  { 1113, "Airship" },
  { 1114, "All Airs" },
  { 1200, "Ferry" },
  { 1300, "Telecabin" },
  { 1301, "Telecabin" },
  { 1302, "Cable Car" },
  { 1303, "Elevator" },
  { 1304, "Chair Lift" },
  { 1305, "Drag Lift" },
  { 1306, "Small Telecabin" },
  { 1307, "All Telecabins" },
  { 1400, "Funicular" },
  { 1401, "Funicular" },
  { 1402, "All Funicular" },
  { 1500, "Taxi" },
  { 1501, "Communal Taxi" },
  { 1502, "Water Taxi" },
  { 1503, "Rail Taxi" },
  { 1504, "Bike Taxi" },
  { 1505, "Licensed Taxi" },
  { 1506, "Private Hire Vehicle" },
  { 1507, "All Taxis" },
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
