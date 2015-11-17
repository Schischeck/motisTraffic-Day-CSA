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

using gtfs_route = std::tuple<cstr, cstr, cstr, cstr, int>;
enum { route_id, agency_id, route_short_name, route_long_name, route_type };
static const column_mapping<gtfs_route> columns = {
    {"route_id", "agency_id", "route_short_name", "route_long_name",
     "route_type"}};

route_map read_routes(loaded_file file, agency_map const& agencies) {
  route_map routes;
  for (auto const& r : read<gtfs_route>(file.content(), columns)) {
    routes.emplace(get<route_id>(r).to_str(),
                   make_unique<route>(
                       agencies.at(get<agency_id>(r).to_str()).get(),
                       get<route_short_name>(r).to_str(),
                       get<route_long_name>(r).to_str(), get<route_type>(r)));
  }
  return routes;
}

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
