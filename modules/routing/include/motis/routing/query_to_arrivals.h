#include <string>
#include <utility>
#include <vector>

#include "motis/protocol/RoutingRequest_generated.h"

#include "motis/routing/arrival.h"

namespace motis {
namespace routing {
namespace query_to_arrivals {
namespace detail {
void add_source_arrival(arrival& source_arrivals, schedule const& sched,
                        MumoEdge const* edge,
                        uint16_t const valid_from = arrival_part::ALWAYS_VALID,
                        bool const periodical_validity = false) {
  auto const st = sched.eva_to_station.find(edge->to_station_eva()->c_str());
  if (st == sched.eva_to_station.end()) {
    return;
  }

  arrival_part a(st->second->index);
  a.price = edge->price();
  a.slot = 0 /* TODO(Mohammad Keyhani) */;
  a.time_cost = edge->duration();
  a.valid_from = valid_from;
  a.periodical_validity = periodical_validity;
  source_arrivals.push_back(a);
}

void process_edge(arrival& source_arrivals, schedule const& sched,
                  AdditionalEdgeWrapper const* additional_edge) {
  if (additional_edge->additional_edge_type() == AdditionalEdge_MumoEdge) {
    auto info =
        reinterpret_cast<MumoEdge const*>(additional_edge->additional_edge());
    if (info->from_station_eva()->str() == "-1") {
      add_source_arrival(source_arrivals, sched, info);
    }
  } else if (additional_edge->additional_edge_type() ==
             AdditionalEdge_TimeDependentMumoEdge) {
    auto info = reinterpret_cast<TimeDependentMumoEdge const*>(
        additional_edge->additional_edge());
    if (info->edge()->from_station_eva()->str() == "-1") {
      uint16_t const valid_from =
          info->periodical_validity() == 1
              ? info->valid_from()
              : unix_to_motistime(sched.schedule_begin_, info->valid_from());
      add_source_arrival(source_arrivals, sched, info->edge(), valid_from,
                         info->periodical_validity() == 1);
    }
  }
}
}  // namespace detail

/* delivers arrival_parts for source and target */
std::pair<arrival, arrival> create_arrivals(
    arrival const& query_from, arrival const& query_to,
    flatbuffers::Vector<flatbuffers::Offset<AdditionalEdgeWrapper>> const*
        additional_edge_infos,
    bool const is_ontrip, schedule const& sched) {
  auto res = std::make_pair(arrival(query_from.begin(), query_from.end()),
                            arrival(query_to.begin(), query_to.end()));

  /* for on-trip, we do not create arrivals (resulting in labels),
   * but insert the edges directly into the graph */
  if (!is_ontrip) {
    for (auto additional_edge : *additional_edge_infos) {
      detail::process_edge(res.first, sched, additional_edge);
    }
  }
  return res;
}
}
}
}
