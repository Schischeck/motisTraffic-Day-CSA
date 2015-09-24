#include "motis/loader/parsers/hrd/stations_parser.h"

#include <map>

#include "parser/arg_parser.h"

#include "motis/core/common/logging.h"
#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/parsers/hrd/files.h"

using namespace parser;
using namespace flatbuffers;
using namespace motis::logging;

namespace motis {
namespace loader {
namespace hrd {

void parse_station_names(loaded_file file,
                         std::map<int, intermediate_station>& stations) {
  scoped_timer timer("parsing station names");
  for_each_line_numbered(file.content, [&](cstr line, int line_number) {
    if (line.len == 0) {
      return;
    } else if (line.len < 14) {
      throw parser_error(file.name, line_number);
    }

    auto eva_num = parse<int>(line.substr(0, size(7)));
    auto name = line.substr(12);

    auto it = std::find(begin(name), end(name), '$');
    if (it != end(name)) {
      name.len = std::distance(begin(name), it);
    }

    stations[eva_num].name = std::string(name.str, name.len);
  });
}

void parse_station_coordinates(loaded_file file,
                               std::map<int, intermediate_station>& stations) {
  scoped_timer timer("parsing station coordinates");
  for_each_line_numbered(file.content, [&](cstr line, int line_number) {
    if (line.len == 0) {
      return;
    } else if (line.len < 30) {
      throw parser_error(file.name, line_number);
    }

    auto eva_num = parse<int>(line.substr(0, size(7)));
    auto& station = stations[eva_num];

    station.lng = parse<double>(line.substr(8, size(10)).trim());
    station.lat = parse<double>(line.substr(19, size(10)).trim());
  });
}

void set_change_times(station_meta_data const& metas,
                      std::map<int, intermediate_station>& stations) {
  scoped_timer timer("set station change times");
  for (auto& station_entry : stations) {
    station_entry.second.change_time =
        metas.get_station_change_time(station_entry.first);
  }
}

std::map<int, intermediate_station> parse_stations(
    loaded_file station_names_file, loaded_file station_coordinates_file,
    station_meta_data const& metas) {
  scoped_timer timer("parsing stations");
  std::map<int, intermediate_station> stations;
  parse_station_names(station_names_file, stations);
  parse_station_coordinates(station_coordinates_file, stations);
  set_change_times(metas, stations);
  return stations;
}

}  // hrd
}  // loader
}  // motis
