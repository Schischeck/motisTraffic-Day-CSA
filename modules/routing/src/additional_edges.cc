#include "motis/routing/additional_edges.h"
#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/schedule.h"

namespace motis {
namespace routing {

std::vector<edge> create_additional_edges(
    flatbuffers::Vector<flatbuffers::Offset<AdditionalEdgeWrapper>> const*
        edge_wrappers,
    schedule const& sched) {
  std::vector<edge> additional_edges;
  std::for_each(
      edge_wrappers->begin(), edge_wrappers->end(),
      [&](AdditionalEdgeWrapper const* additional_edge) {
        /* mumo */
        switch (additional_edge->additional_edge_type()) {
          case AdditionalEdge_MumoEdge: {
            auto info = reinterpret_cast<MumoEdge const*>(
                additional_edge->additional_edge());
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
            break;
          }
          case AdditionalEdge_TimeDependentMumoEdge: {
            auto info = reinterpret_cast<TimeDependentMumoEdge const*>(
                additional_edge->additional_edge());
            auto from_station_node = sched.eva_to_station.find(
                info->edge()->from_station_eva()->c_str());
            auto to_station_node = sched.eva_to_station.find(
                info->edge()->to_station_eva()->c_str());
            if (from_station_node != sched.eva_to_station.end() &&
                to_station_node != sched.eva_to_station.end()) {
              /* todo: read validity interval and store it in the edge */
              additional_edges.push_back(make_time_dependent_mumo_edge(
                  sched.station_nodes[from_station_node->second->index].get(),
                  sched.station_nodes[to_station_node->second->index].get(),
                  info->edge()->duration(), info->edge()->price(),
                  0 /* todo: slot */));
            }
            break;
          }
          /* hotel edges */
          case AdditionalEdge_HotelEdge: {
            auto info = reinterpret_cast<HotelEdge const*>(
                additional_edge->additional_edge());
            auto const station_node =
                sched.eva_to_station.find(info->station_eva()->c_str());
            if (station_node != sched.eva_to_station.end()) {
              additional_edges.push_back(make_hotel_edge(
                  sched.station_nodes[station_node->second->index].get(),
                  info->earliest_checkout_time(), info->min_stay_duration(),
                  info->price()));
            }
            break;
          }
          default: break;
        }
      });
  return additional_edges;
}
}  // namespace routing
}  // namespace motis
