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

using route = std::tuple<int, parser::cstr, parser::cstr, int>;
enum { route_id, route_short_name, route_long_name, route_type };
static const parser::column_mapping route_columns = {
    {"route_id", "route_short_name", "route_long_name", "route_type"}};

parser::cstr route_name(route const& r) {}

std::vector<Offset<String>> read_stations(loaded_file file,
                                          FlatBufferBuilder& b) {
  std::vector<Offset<String>> categories;
  for (auto const& route : parser::read<route>(file.content, route_columns)) {
  }
  return categories;
}

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
