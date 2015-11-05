#pragma once

#include <map>

#include "motis/loader/hrd/parser/stations_parser.h"

#include "motis/schedule-format/Station_generated.h"

namespace motis {
namespace loader {
namespace hrd {

struct station_builder {
  station_builder(std::map<int, intermediate_station>);

  flatbuffers::Offset<Station> get_or_create_station(
      int, flatbuffers::FlatBufferBuilder&);

  std::map<int, intermediate_station> hrd_stations_;
  std::map<int, flatbuffers::Offset<Station>> fbs_stations_;
};

}  // hrd
}  // loader
}  // motis
