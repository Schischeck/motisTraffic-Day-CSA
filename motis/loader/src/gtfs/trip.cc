#include "motis/loader/parsers/gtfs/trip.h"

#include <tuple>
#include <algorithm>

#include "parser/csv.h"

#include "motis/loader/util.h"

using namespace parser;
using std::get;

namespace motis {
namespace loader {
namespace gtfs {

enum { route_id, service_id, trip_id, trip_headsign };
using gtfs_trip = std::tuple<cstr, cstr, cstr, cstr>;
static const column_mapping<gtfs_trip> trip_columns = {
    {"route_id", "service_id", "trip_id", "trip_headsign"}};

std::map<std::string, trip> read_trips(loaded_file file) {
  std::map<std::string, trip> trips;
  for (auto const& t : read<gtfs_trip>(file.content_, trip_columns)) {
    trips.insert(std::make_pair(
        get<trip_id>(t).to_str(),
        trip{get<route_id>(t).to_str(), get<service_id>(t).to_str(),
             get<trip_headsign>(t).to_str()}));
  }
  return trips;
}

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
