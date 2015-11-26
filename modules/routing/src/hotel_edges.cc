#include "motis/routing/hotel_edges.h"

#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/schedule.h"

namespace motis {
namespace routing {

std::vector<edge> create_hotel_edges(
    flatbuffers::Vector<flatbuffers::Offset<HotelEdge>> const*
        additional_edge_infos,
    schedule const& sched) {
  std::vector<edge> hotel_edges;
  std::for_each(
      additional_edge_infos->begin(), additional_edge_infos->end(),
      [&](HotelEdge const* info) {
        auto const station_node =
            sched.eva_to_station.find(info->station_eva()->c_str());
        if (station_node != sched.eva_to_station.end()) {
          hotel_edges.push_back(make_hotel_edge(
              sched.station_nodes[station_node->second->index].get(),
              info->checkout_time(), info->min_stay_duration(), info->price()));
        }
      });
  return hotel_edges;
}
}  // namespace routing
}  // namespace motis
