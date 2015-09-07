#include "motis/loader/parsers/hrd/footpath_builder.h"

#include "motis/loader/util.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace flatbuffers;

Offset<Vector<Offset<Footpath>>> build_footpaths(
    std::vector<station_meta_data::footpath> const& footpaths,
    std::map<int, flatbuffers::Offset<Station>> const& fbs_stations,
    FlatBufferBuilder& b) {
  std::vector<Offset<Footpath>> fbs_footpaths;
  for (auto const& f : footpaths) {
    auto from_station_it = fbs_stations.find(f.from_eva_num);
    auto to_station_it = fbs_stations.find(f.to_eva_num);
    if (from_station_it != end(fbs_stations) &&
        to_station_it != end(fbs_stations)) {
      fbs_footpaths.push_back(CreateFootpath(
          b, from_station_it->second, to_station_it->second, f.duration));
    }
  }
  return b.CreateVector(fbs_footpaths);
}

}  // hrd
}  // loader
}  // motis
