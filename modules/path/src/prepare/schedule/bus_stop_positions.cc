#include "motis/path/prepare/schedule/bus_stop_positions.h"

#include "boost/algorithm/string/predicate.hpp"

#include "geo/point_rtree.h"

#include "motis/path/prepare/fbs/use_64bit_flatbuffers.h"
#include "motis/path/prepare/osm_util.h"

#include "motis/schedule-format/Schedule_generated.h"

using namespace geo;

namespace motis {
namespace path {

std::map<std::string, std::vector<latlng>> find_bus_stop_positions(
    motis::loader::Schedule const* sched, std::string const& osm_file) {
  std::string const stop_position = "stop_position";
  std::string const yes = "yes";

  auto const rtree = make_point_rtree(*sched->stations(), [](auto&& s) {
    return latlng{s->lng(), s->lat()};
  });

  std::map<std::string, std::vector<latlng>> result;
  foreach_osm_node(osm_file, [&](auto&& node) {
    if (stop_position != node.get_value_by_key("public_transport", "") ||
        yes != node.get_value_by_key("bus", "")) {
      return;
    }

    auto const lat = node.location().lat();
    auto const lng = node.location().lon();

    for (auto const& station_index : rtree.in_radius({lat, lng}, 100)) {
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

}  // namespace path
}  // namespace motis
