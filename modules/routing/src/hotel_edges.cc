#include "motis/routing/hotel_edges.h"

#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/schedule.h"

namespace motis {
namespace routing {

std::vector<edge> create_hotel_edges(
    flatbuffers::Vector<flatbuffers::Offset<AdditionalEdgeWrapper>> const*
        additional_edge_infos,
    schedule const& sched) {
  std::vector<edge> hotel_edges;
  std::for_each(
      additional_edge_infos->begin(), additional_edge_infos->end(),
      [&](AdditionalEdgeWrapper const* additional_edge) {
        if (additional_edge->additional_edge_type() ==
            AdditionalEdge_HotelEdge) {
          auto info = (HotelEdge const*)additional_edge->additional_edge();
          auto const station_node =
              sched.eva_to_station.find(info->station_eva()->c_str());
          if (station_node != sched.eva_to_station.end()) {
            hotel_edges.push_back(make_hotel_edge(
                sched.station_nodes[station_node->second->index].get(),
                info->earliest_checkout_time(), info->min_stay_duration(),
                info->price()));
          }
        }
      });
  return hotel_edges;
}
}  // namespace routing
}  // namespace motis
