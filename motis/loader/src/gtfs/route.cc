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
static const parser::column_mapping<route> route_columns = {
    {"route_id", "route_short_name", "route_long_name", "route_type"}};

std::vector<Offset<String>> read_route(loaded_file file, FlatBufferBuilder& b) {
  std::vector<Offset<String>> categories;
  for (auto const& r : parser::read<route>(file.content, route_columns)) {
    categories.push_back(to_fbs_string(b, get<route_short_name>(r)));
  }
  return categories;
}

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
