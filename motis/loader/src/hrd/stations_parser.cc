#include "motis/loader/parsers/hrd/stations_parser.h"

#include <map>

#include "../../include/motis/loader/parsers/hrd/change_times_parser.h"
#include "parser/arg_parser.h"

#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/parsers/hrd/files.h"

using namespace parser;
using namespace flatbuffers;

namespace motis {
namespace loader {
namespace hrd {

struct station {
  cstr name;
  double lat, lng;
};

void parse_station_names(loaded_file file, std::map<int, station>& stations) {
  for_each_line_numbered(file.content, [&](cstr line, int line_number) {
    if (line.len == 0) {
      return;
    } else if (line.len < 14) {
      throw parser_error(file.name, line_number);
    }

    auto eva_num = parse<int>(line.substr(0, size(7)));
    stations[eva_num].name = line.substr(12);
  });
}

void parse_station_coordinates(loaded_file file,
                               std::map<int, station>& stations) {
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

std::map<int, Offset<Station>> parse_stations(
    loaded_file station_names_file, loaded_file station_coordinates_file,
    loaded_file ds100_mappings_file, flatbuffers::FlatBufferBuilder& b) {
  std::map<int, station> stations_map;
  parse_station_names(station_names_file, stations_map);
  parse_station_coordinates(station_coordinates_file, stations_map);
  change_times const ic_times(ds100_mappings_file);

  std::map<int, Offset<Station>> stations;
  for (auto const& station_entry : stations_map) {
    auto& eva_num = station_entry.first;
    auto& station = station_entry.second;
    stations.insert(std::make_pair(
        eva_num,
        CreateStation(b, to_fbs_string(b, std::to_string(eva_num)),
                      to_fbs_string(b, station.name, ENCODING), station.lat,
                      station.lng, ic_times.get_interchange_time(eva_num))));
  }
  return stations;
}

}  // hrd
}  // loader
}  // motis
