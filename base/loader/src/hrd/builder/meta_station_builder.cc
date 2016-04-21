#include "motis/loader/hrd/builder/meta_station_builder.h"

#include <iostream>

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
    for (auto const& e : m.equivalent_) {
      auto it = fbs_stations.find(e);
      if (it == end(fbs_stations)) {
        continue;
      }
      fbs_equivalent.push_back(it->second);
    }

    if (fbs_equivalent.size() == 0) {
      continue;
    }

    auto it = fbs_stations.find(m.eva_);
    if (it == end(fbs_stations)) {
      continue;
    }
    fbs_meta_stations.push_back(
        CreateMetaStation(fbb, it->second, fbb.CreateVector(fbs_equivalent)));
  }
  return fbb.CreateVector(fbs_meta_stations);
}

}  // hrd
}  // loader
}  // motis
