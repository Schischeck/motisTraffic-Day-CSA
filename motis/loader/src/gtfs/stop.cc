#include "motis/loader/parsers/gtfs/stop.h"

#include <tuple>
#include <algorithm>

#include "parser/csv.h"

#include "motis/loader/util.h"

using namespace flatbuffers;
using std::get;

namespace motis {
namespace loader {
namespace gtfs {

enum { stop_id, stop_name, stop_lat, stop_lon };
using stop = std::tuple<parser::cstr, parser::cstr, float, float>;
static const parser::column_mapping<stop> stop_columns = {
    {"stop_id", "stop_name", "stop_lat", "stop_lon"}};

std::map<std::string, Offset<Station>> read_stations(loaded_file file,
                                                     FlatBufferBuilder& b) {
  std::vector<stop> stops = parser::read<stop>(file.content, stop_columns);
  std::map<std::string, Offset<Station>> stations;
  for (auto const& s : stops) {
    auto id = get<stop_id>(s);
    stations.insert(
        std::make_pair(std::string(id.str, id.len),
                       CreateStation(b, to_fbs_string(b, id),
                                     to_fbs_string(b, get<stop_name>(s)),
                                     get<stop_lat>(s), get<stop_lon>(s))));
  }
  return stations;
}

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
