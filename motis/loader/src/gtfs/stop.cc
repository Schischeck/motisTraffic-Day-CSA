#include "motis/loader/gtfs/stop.h"

#include <tuple>
#include <algorithm>

#include "parser/csv.h"

#include "motis/loader/util.h"

using namespace parser;
using namespace flatbuffers;
using std::get;

namespace motis {
namespace loader {
namespace gtfs {

enum { stop_id, stop_name, stop_lat, stop_lon };
using stop = std::tuple<cstr, cstr, float, float>;
static const column_mapping<stop> stop_columns = {
    {"stop_id", "stop_name", "stop_lat", "stop_lon"}};

std::map<std::string, Offset<Station>> read_stations(loaded_file file,
                                                     FlatBufferBuilder& b) {
  std::vector<stop> stops = read<stop>(file.content(), stop_columns);
  std::map<std::string, Offset<Station>> stations;
  for (auto const& s : stops) {
    stations.insert(
        std::make_pair(get<stop_id>(s).to_str(),
                       CreateStation(b, to_fbs_string(b, get<stop_id>(s)),
                                     to_fbs_string(b, get<stop_name>(s)),
                                     get<stop_lat>(s), get<stop_lon>(s))));
  }
  return stations;
}

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
