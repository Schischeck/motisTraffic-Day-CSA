#include "motis/loader/hrd/builder/meta_station_builder.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace flatbuffers;

Offset<Vector<Offset<MetaStation>>> create_meta_stations(
    std::set<station_meta_data::meta_station> const& hrd_meta_stations,
    std::map<int, flatbuffers::Offset<Station>> const& fbs_stations,
    FlatBufferBuilder& fbb) {
  std::vector<Offset<MetaStation>> fbs_meta_stations;
  for (auto const& m : hrd_meta_stations) {
    std::vector<Offset<Station>> fbs_equivalent;
    for (auto const& e : m.equivalent) {
      auto it = fbs_stations.find(e);
      fbs_equivalent.push_back(it->second);
    }

    auto it = fbs_stations.find(m.eva);
    fbs_meta_stations.push_back(
        CreateMetaStation(fbb, it->second, fbb.CreateVector(fbs_equivalent)));
  }
  return fbb.CreateVector(fbs_meta_stations);
}

}  // hrd
}  // loader
}  // motis
