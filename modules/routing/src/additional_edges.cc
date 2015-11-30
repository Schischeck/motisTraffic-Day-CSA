#include "motis/routing/additional_edges.h"
#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/schedule.h"

namespace motis {
namespace routing {

std::vector<edge> create_additional_edges(
    flatbuffers::Vector<flatbuffers::Offset<AdditionalEdgeWrapper>> const*
        additional_edge_infos,
    schedule const& sched) {
  std::vector<edge> additional_edges;
  std::for_each(
      additional_edge_infos->begin(), additional_edge_infos->end(),
      [&](AdditionalEdgeWrapper const* additional_edge) {
        /* mumo like taxi */
        if (additional_edge->additional_edge_type() ==
            AdditionalEdge_MumoEdge) {
          auto info = (MumoEdge const*)additional_edge->additional_edge();
          auto from_station_node =
              sched.eva_to_station.find(info->from_station_eva()->c_str());
          auto to_station_node =
              sched.eva_to_station.find(info->to_station_eva()->c_str());
          if (from_station_node != sched.eva_to_station.end() &&
              to_station_node != sched.eva_to_station.end()) {
            additional_edges.push_back(make_mumo_edge(
                sched.station_nodes[from_station_node->second->index].get(),
                sched.station_nodes[to_station_node->second->index].get(),
                info->duration(), info->price(), 0 /* todo: slot */));
          }
        }
        /* hotel edges */
        else if (additional_edge->additional_edge_type() ==
                 AdditionalEdge_HotelEdge) {
          auto info = (HotelEdge const*)additional_edge->additional_edge();
          auto const station_node =
              sched.eva_to_station.find(info->station_eva()->c_str());
          if (station_node != sched.eva_to_station.end()) {
            additional_edges.push_back(make_hotel_edge(
                sched.station_nodes[station_node->second->index].get(),
                info->earliest_checkout_time(), info->min_stay_duration(),
                info->price()));
          }
        }
      });
  return additional_edges;
}
}  // namespace routing
}  // namespace motis
