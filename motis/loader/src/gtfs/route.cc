#include "motis/loader/parsers/gtfs/route.h"

#include <tuple>
#include <algorithm>

#include "parser/csv.h"

#include "motis/loader/util.h"

using namespace flatbuffers;
using std::get;

namespace motis {
namespace loader {
namespace gtfs {

using gtfs_route =
    std::tuple<parser::cstr, parser::cstr, parser::cstr, parser::cstr, int>;
enum { route_id, agency_id, route_short_name, route_long_name, route_type };
static const parser::column_mapping<gtfs_route> route_columns = {
    {"route_id", "agency_id", "route_short_name", "route_long_name",
     "route_type"}};

std::map<std::string, route> read_routes(loaded_file file) {
  std::map<std::string, route> routes;
  for (auto const& r : parser::read<gtfs_route>(file.content, route_columns)) {
    auto const& id = get<route_id>(r);
    auto const& short_name = get<route_short_name>(r);
    auto const& long_name = get<route_long_name>(r);
    auto const& agency = get<agency_id>(r);
    routes.insert(std::make_pair(
        std::string(id.str, id.len),
        route{std::string(agency.str, agency.len),
              std::string(short_name.str, short_name.len),
              std::string(long_name.str, long_name.len), get<route_type>(r)}));
  }
  return routes;
}

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
