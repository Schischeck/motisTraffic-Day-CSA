#include "motis/loader/parsers/gtfs/stop.h"

#include <tuple>
#include <array>

#include "parser/csv.h"

#include "motis/loader/parsers/gtfs/files.h"
#include "motis/loader/util.h"

using namespace flatbuffers;
using namespace parser;
using std::get;

namespace motis {
namespace loader {
namespace gtfs {

enum { stop_id, stop_name, stop_lat, stop_lon };
using stop = std::tuple<cstr, cstr, float, float>;
static const column_mapping<stop> stop_columns = {
    {"stop_id", "stop_name", "stop_lat", "stop_lon"}};

std::vector<Offset<Station>> read_stations(loaded_file file,
                                           FlatBufferBuilder& b) {
  std::vector<Offset<Station>> stations;
  std::vector<stop> stops = read<stop>(file.content, stop_columns);
  for (auto const& stop : stops) {
    stations.push_back(CreateStation(b, to_fbs_string(b, get<stop_id>(stop)),
                                     to_fbs_string(b, get<stop_name>(stop)),
                                     get<stop_lat>(stop), get<stop_lon>(stop)));
  }
  return stations;
}

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
