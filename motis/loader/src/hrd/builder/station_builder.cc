#include "motis/loader/hrd/builder/station_builder.h"

#include "motis/loader/util.h"
#include "motis/loader/hrd/files.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace flatbuffers;

station_builder::station_builder(
    std::map<int, intermediate_station> hrd_stations)
    : hrd_stations_(std::move(hrd_stations)){};

Offset<Station> station_builder::get_or_create_station(int eva_num,
                                                       FlatBufferBuilder& fbb) {
  return get_or_create(fbs_stations_, eva_num, [&]() {
    auto it = hrd_stations_.find(eva_num);
    verify(it != end(hrd_stations_), "missing station: %d", eva_num);
    return CreateStation(
        fbb, to_fbs_string(fbb, std::to_string(eva_num)),
        to_fbs_string(fbb, it->second.name, ENCODING), it->second.lat,
        it->second.lng, it->second.change_time,
        fbb.CreateVector(transform_to_vec(
            begin(it->second.ds100), end(it->second.ds100),
            [&](std::string const& s) { return fbb.CreateString(s); })));
  });
}

}  // hrd
}  // loader
}  // motis
