#include "motis/loader/hrd/builder/footpath_builder.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace flatbuffers64;

Offset<Vector<Offset<Footpath>>> create_footpaths(
    std::set<station_meta_data::footpath> const& hrd_footpaths,
    std::map<int, flatbuffers64::Offset<Station>> const& fbs_stations,
    FlatBufferBuilder& fbb) {
  std::vector<Offset<Footpath>> fbs_footpaths;
  for (auto const& f : hrd_footpaths) {
    auto from_station_it = fbs_stations.find(f.from_eva_num_);
    auto to_station_it = fbs_stations.find(f.to_eva_num_);
    if (from_station_it != end(fbs_stations) &&
        to_station_it != end(fbs_stations)) {
      fbs_footpaths.push_back(CreateFootpath(
          fbb, from_station_it->second, to_station_it->second, f.duration_));
    }
  }
  return fbb.CreateVector(fbs_footpaths);
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
