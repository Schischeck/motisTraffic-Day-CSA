#include "motis/routes/prepare/bus_stop_positions.h"

#include <iostream>

#include "motis/routes/prepare/osm_util.h"
#include "motis/routes/prepare/point_rtree.h"

using namespace geo;

namespace motis {
namespace routes {

std::map<std::string, std::vector<latlng>> find_bus_stop_positions(
    motis::loader::Schedule const* sched, std::string const& osm_file) {
  std::string const stop_position = "stop_position";
  std::string const yes = "yes";

  auto rtree = make_point_rtree(*sched->stations(), [](auto&& s) {
    return point_rtree::point{s->lng(), s->lat()};
  });

  std::map<std::string, std::vector<latlng>> result;
  foreach_osm_node(osm_file, [&](auto&& node) {
    if (stop_position != node.get_value_by_key("public_transport", "") ||
        yes != node.get_value_by_key("bus", "")) {
      return;
    }

    auto const lat = node.location().lat();
    auto const lng = node.location().lon();

    for (auto const& station_index : rtree.in_radius(lat, lng, 100)) {
      auto const station = sched->stations()->Get(station_index);
      auto const station_id = station->id()->str();
      if (boost::algorithm::starts_with(station_id, "80")) {
        continue;
      }

      result[station_id].emplace_back(lat, lng);
      break;
    }
  });

  return result;
}

flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<StopPositions>>>
write_stop_positions(
    flatbuffers::FlatBufferBuilder& fbb,
    std::map<std::string, std::vector<latlng>> const& stop_positions) {
  std::vector<flatbuffers::Offset<StopPositions>> result;

  for (auto const& pair : stop_positions) {
    std::vector<Position> positions;
    for (auto const& p : pair.second) {
      positions.emplace_back(p.lat_, p.lng_);
    }
    result.emplace_back(
        CreateStopPositions(fbb, fbb.CreateString(pair.first),
                            fbb.CreateVectorOfStructs(positions)));
  }

  return fbb.CreateVector(result);
}

}  // namespace routes
}  // namespace motis
