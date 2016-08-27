#include "motis/routes/prepare/bus_stop_positions.h"

#include <iostream>

#include "motis/routes/prepare/osm_util.h"
#include "motis/routes/prepare/point_rtree.h"

namespace motis {
namespace routes {

flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<BusStopPosition>>>
find_bus_stop_positions(flatbuffers::FlatBufferBuilder& fbb,
                        motis::loader::Schedule const* sched,
                        std::string const& osm_file) {
  std::string const stop_position = "stop_position";
  std::string const yes = "yes";

  auto rtree = make_point_rtree(*sched->stations(), [](auto&& s) {
    return point_rtree::point{s->lng(), s->lat()};
  });

  std::vector<flatbuffers::Offset<BusStopPosition>> bus_stop_positions;
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

      bus_stop_positions.push_back(
          CreateBusStopPosition(fbb, fbb.CreateString(station_id), lat, lng));
      break;
    }
  });

  std::cout << "positions" << bus_stop_positions.size() << std::endl;
  return fbb.CreateVector(bus_stop_positions);
}

}  // namespace routes
}  // namespace motis