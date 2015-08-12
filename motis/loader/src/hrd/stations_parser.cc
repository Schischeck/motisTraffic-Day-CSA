#include "motis/loader/parsers/hrd/stations_parser.h"

#include <map>

#include "parser/arg_parser.h"

#include "motis/loader/util.h"

#include "motis/loader/parser_error.h"

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
    station.lng = parse<double>(line.substr(8, size(10)));
    station.lat = parse<double>(line.substr(19, size(10)));
  });
}

std::map<int, Offset<Station>> parse_stations(loaded_file stations_file,
                                              loaded_file coordinates_file,
                                              FlatBufferBuilder& b) {
  std::map<int, station> stations_map;
  parse_station_names(stations_file, stations_map);
  parse_station_coordinates(coordinates_file, stations_map);

  std::map<int, Offset<Station>> stations;
  for (auto const& station_entry : stations_map) {
    auto& eva_num = station_entry.first;
    auto& station = station_entry.second;
    stations.emplace(eva_num, CreateStation(b, to_fbs_string(b, station.name),
                                            eva_num, station.lat, station.lng));
  }
  return stations;
}

}  // hrd
}  // loader
}  // motis
