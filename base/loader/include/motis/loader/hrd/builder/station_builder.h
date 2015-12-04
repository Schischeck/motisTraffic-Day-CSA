#pragma once

#include <map>

#include "motis/loader/hrd/model/timezones.h"
#include "motis/loader/hrd/parser/stations_parser.h"

#include "motis/schedule-format/Station_generated.h"

namespace motis {
namespace loader {
namespace hrd {

struct station_builder {
  station_builder(std::map<int, intermediate_station>, timezones);

  flatbuffers::Offset<Station> get_or_create_station(
      int, flatbuffers::FlatBufferBuilder&);

  std::map<int, intermediate_station> hrd_stations_;
  timezones timezones_;
  std::map<int, flatbuffers::Offset<Station>> fbs_stations_;
  std::map<timezone_entry const*, flatbuffers::Offset<Timezone>> fbs_timezones_;
};

}  // hrd
}  // loader
}  // motis
