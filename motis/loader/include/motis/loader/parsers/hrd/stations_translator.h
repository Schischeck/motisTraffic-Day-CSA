#pragma once

#include <map>

#include "flatbuffers/flatbuffers.h"

#include "motis/loader/loaded_file.h"
#include "motis/loader/parsers/hrd/stations_parser.h"
#include "motis/loader/parsers/hrd/station_meta_data_parser.h"
#include "motis/schedule-format/Station_generated.h"

namespace motis {
namespace loader {
namespace hrd {

struct stations_translator {
  stations_translator(std::map<int, intermediate_station> const& stations,
                      flatbuffers::FlatBufferBuilder& builder)
      : stations_(stations), builder_(builder){};

  flatbuffers::Offset<Station> get_or_create_station(int eva_num);

  std::map<int, intermediate_station> const& stations_;
  std::map<int, flatbuffers::Offset<Station>> fbs_stations_;
  flatbuffers::FlatBufferBuilder& builder_;
};

}  // hrd
}  // loader
}  // motis
