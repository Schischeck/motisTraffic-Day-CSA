#include "motis/loader/hrd/builder/footpath_builder.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace flatbuffers64;

Offset<Vector<Offset<Footpath>>> create_footpaths(
    std::set<station_meta_data::footpath> const& hrd_footpaths,
    station_builder& stb, FlatBufferBuilder& fbb) {
  std::vector<Offset<Footpath>> fbs_footpaths;
  for (auto const& f : hrd_footpaths) {
    fbs_footpaths.push_back(
        CreateFootpath(fbb,  //
                       stb.get_or_create_station(f.from_eva_num_, fbb),
                       stb.get_or_create_station(f.to_eva_num_, fbb),  //
                       f.duration_));
  }
  return fbb.CreateVector(fbs_footpaths);
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
