#include "motis/loader/parsers/gtfs/stop.h"

#include <tuple>
#include <array>

#include "parser/csv.h"

#include "motis/loader/parsers/gtfs/files.h"
#include "motis/loader/util.h"

using namespace flatbuffers;
using namespace parser;
namespace fs = boost::filesystem;
using std::get;

namespace motis {
namespace loader {
namespace gtfs {

using stop = std::tuple<int,  // stop_id
                        parser::cstr,  // stop_code
                        parser::cstr,  // stop_name
                        parser::cstr,  // stop_desc
                        float,  // stop_lat
                        float,  // stop_lon
                        int,  // zone_id
                        parser::cstr,  // stop_url
                        int,  // location_type
                        int  // parent_station
                        >;

static const std::array<parser::cstr, std::tuple_size<stop>::value>
    stop_columns = {{"stop_id", "stop_code", "stop_name", "stop_desc",
                     "stop_lat", "stop_lon", "zone_id", "stop_url",
                     "location_type", "parent_statiom"}};

enum {
  stop_id,
  stop_code,
  stop_name,
  stop_desc,
  stop_lat,
  stop_lon,
  zone_id,
  stop_url,
  location_type,
  parent_station
};

std::vector<Offset<Station>> read_stations(fs::path const& path,
                                           FlatBufferBuilder& b) {
  std::vector<Offset<Station>> stations;
  auto stops = read_file<stop>(path / STOPS_FILE, stop_columns);
  for (auto const& stop : stops) {
    stations.push_back(CreateStation(b, to_fbs_string(b, get<stop_name>(stop)),
                                     get<stop_id>(stop), get<stop_lat>(stop),
                                     get<stop_lon>(stop)));
  }
  return stations;
}

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
