#include "motis/loader/gtfs/stop.h"

#include <algorithm>
#include <tuple>

#include "parser/csv.h"

#include "motis/loader/util.h"

using namespace parser;
using namespace flatbuffers64;
using std::get;

namespace motis {
namespace loader {
namespace gtfs {

enum { stop_id, stop_name, stop_lat, stop_lon };
using gtfs_stop = std::tuple<cstr, cstr, float, float>;
static const column_mapping<gtfs_stop> columns = {
    {"stop_id", "stop_name", "stop_lat", "stop_lon"}};

stop_map read_stops(loaded_file file) {
  stop_map stops;
  for (auto const& s : read<gtfs_stop>(file.content(), columns)) {
    stops.emplace(get<stop_id>(s).to_str(),
                  std::make_unique<stop>(get<stop_id>(s).to_str(),
                                         get<stop_name>(s).to_str(),
                                         get<stop_lat>(s), get<stop_lon>(s)));
  }
  return stops;
}

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
