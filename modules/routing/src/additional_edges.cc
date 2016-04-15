#include "motis/routing/additional_edges.h"
#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/schedule.h"

namespace motis {
namespace routing {
namespace detail {

std::pair<bool, edge> create_mumo_edge(
    AdditionalEdgeWrapper const* additional_edge, schedule const& sched,
    std::function<bool(MumoEdge const* edge)> skip_edge) {
  auto info =
      reinterpret_cast<MumoEdge const*>(additional_edge->additional_edge());
  auto from_station_node =
      sched.eva_to_station.find(info->from_station_eva()->c_str());
  auto to_station_node =
      sched.eva_to_station.find(info->to_station_eva()->c_str());
  if (from_station_node == sched.eva_to_station.end() ||
      to_station_node == sched.eva_to_station.end() || skip_edge(info)) {
    return std::make_pair(false, edge());
  }

  return std::make_pair(
      true, make_mumo_edge(
                sched.station_nodes[from_station_node->second->index].get(),
                sched.station_nodes[to_station_node->second->index].get(),
                info->duration(), info->price(),
                0 /* TODO(Mohammad Keyhani) slot */));
}

std::pair<bool, edge> create_time_dependent_mumo_edge(
    AdditionalEdgeWrapper const* additional_edge, schedule const& sched,
    std::function<bool(MumoEdge const* edge)> skip_edge) {
  auto info = reinterpret_cast<TimeDependentMumoEdge const*>(
      additional_edge->additional_edge());
  auto from_station_node =
      sched.eva_to_station.find(info->edge()->from_station_eva()->c_str());
  auto to_station_node =
      sched.eva_to_station.find(info->edge()->to_station_eva()->c_str());
  if (from_station_node == sched.eva_to_station.end() ||
      to_station_node == sched.eva_to_station.end() ||
      skip_edge(info->edge())) {
    return std::make_pair(false, edge());
  }

  uint16_t const valid_from =
      info->periodical_validity() == 1
          ? static_cast<uint16_t>(info->valid_from())
          : unix_to_motistime(sched.schedule_begin_, info->valid_from());
  uint16_t const valid_to =
      info->periodical_validity() == 1
          ? static_cast<uint16_t>(info->valid_to())
          : unix_to_motistime(sched.schedule_begin_, info->valid_to());

  return std::make_pair(
      true,
      make_time_dependent_mumo_edge(
          sched.station_nodes[from_station_node->second->index].get(),
          sched.station_nodes[to_station_node->second->index].get(), valid_from,
          valid_to, info->periodical_validity() == 1, info->edge()->duration(),
          info->edge()->price(), 0 /* TODO(Mohammad Keyhani) slot */));
}

std::pair<bool, edge> create_hotel_edge(
    AdditionalEdgeWrapper const* additional_edge, schedule const& sched) {
  auto info =
      reinterpret_cast<HotelEdge const*>(additional_edge->additional_edge());
  auto const station_node =
      sched.eva_to_station.find(info->station_eva()->c_str());
  if (station_node == sched.eva_to_station.end()) {
    return std::make_pair(false, edge());
  }

  return std::make_pair(
      true,
      make_hotel_edge(sched.station_nodes[station_node->second->index].get(),
                      info->earliest_checkout_time(), info->min_stay_duration(),
                      info->price()));
}
}

std::vector<edge> create_additional_edges(
    flatbuffers::Vector<flatbuffers::Offset<AdditionalEdgeWrapper>> const*
        additional_edge_infos,
    bool const is_ontrip, schedule const& sched) {
  /* for pre-trip, we do not insert the edges at source directly into the graph,
   * but create arrivals (resulting in labels) */
  auto skip_edge = [is_ontrip](MumoEdge const* edge) -> bool {
    return !is_ontrip && (edge->from_station_eva()->str() == "-1");
  };

  std::vector<edge> additional_edges;
  for (auto info : *additional_edge_infos) {
    std::pair<bool, edge> e;
    e.first = false;
    auto const type = info->additional_edge_type();
    if (type == AdditionalEdge_MumoEdge) {
      e = detail::create_mumo_edge(info, sched, skip_edge);
    } else if (type == AdditionalEdge_TimeDependentMumoEdge) {
      e = detail::create_time_dependent_mumo_edge(info, sched, skip_edge);
    } else if (type == AdditionalEdge_HotelEdge) {
      e = detail::create_hotel_edge(info, sched);
    }
    if (e.first) {
      additional_edges.push_back(e.second);
    }
  }
  return additional_edges;
}
}  // namespace routing
}  // namespace motis
