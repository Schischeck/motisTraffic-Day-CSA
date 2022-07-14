#include "motis/loader/hrd/builder/meta_station_builder.h"

#include "motis/core/common/logging.h"

#include <iostream>
#include <optional>

#include "utl/pipes/all.h"
#include "utl/pipes/remove_if.h"
#include "utl/pipes/transform.h"
#include "utl/pipes/vec.h"
#include "utl/to_vec.h"

using namespace motis::logging;

namespace motis {
namespace loader {
namespace hrd {

using namespace flatbuffers64;

Offset<Vector<Offset<MetaStation>>> create_meta_stations(
    std::set<station_meta_data::meta_station> const& hrd_meta_stations,
    std::map<int, flatbuffers64::Offset<Station>> const& fbs_stations,
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

    if (fbs_equivalent.empty()) {
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

Offset<Vector<Offset<MetaStation>>> create_meta_stations(
    std::set<station_meta_data::meta_station> const& hrd_meta_stations,
    station_builder& sb, FlatBufferBuilder& fbb) {
  using namespace utl;
  return fbb.CreateVector(
      all(hrd_meta_stations)  //
      | remove_if([&](auto&& m) { return m.equivalent_.empty(); })  //
      |
      transform([&](auto&& m) -> std::optional<Offset<MetaStation>> {
        try {
          return std::make_optional(CreateMetaStation(
              fbb, sb.get_or_create_station(m.eva_, fbb),
              fbb.CreateVector(utl::to_vec(
                  get_equivalent_stations(m, hrd_meta_stations), [&](auto&& e) {
                    return sb.get_or_create_station(e, fbb);
                  }))));
        } catch (std::exception const& e) {
          LOG(error) << "meta station error: " << e.what();
          return std::nullopt;
        }
      })  //
      | remove_if([](auto&& opt) { return !opt.has_value(); })  //
      | transform([](auto&& opt) { return *opt; })  //
      | vec());
}

void add_equivalent_stations(
    std::vector<int>& visited, int stationeva,
    std::set<station_meta_data::meta_station> const& hrd_meta_stations) {
  auto const& s = hrd_meta_stations.find({stationeva, {}});
  if (s == hrd_meta_stations.end()) {
    return;
  }
  for (auto const& e : s->equivalent_) {
    if (std::find(begin(visited), end(visited), e) == end(visited)) {
      visited.push_back(e);
      add_equivalent_stations(visited, e, hrd_meta_stations);
    }
  }
}

std::vector<int> get_equivalent_stations(
    station_meta_data::meta_station const& s,
    std::set<station_meta_data::meta_station> const& hrd_meta_stations) {
  auto stations = s.equivalent_;
  for (auto const& station : s.equivalent_) {
    add_equivalent_stations(stations, station, hrd_meta_stations);
  }
  return stations;
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
