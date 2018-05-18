#pragma once

#include <vector>

#include "motis/core/common/hash_map.h"
#include "motis/core/schedule/constant_graph.h"
#include "motis/core/schedule/schedule.h"

#include "motis/routing/label/criteria/transfers.h"
#include "motis/routing/label/criteria/travel_time.h"

namespace motis {
namespace routing {

struct lower_bounds {
  lower_bounds(
      schedule const& sched,  //
      constant_graph const& travel_time_graph,
      constant_graph const& transfers_graph,  //
      int goal,
      hash_map<int, std::vector<simple_edge>> const&
          additional_travel_time_edges,
      hash_map<int, std::vector<simple_edge>> const& additional_transfers_edges)
      : travel_time_(travel_time_graph, goal, additional_travel_time_edges),
        transfers_(transfers_graph, goal, additional_transfers_edges,
                   map_interchange_graph_node(sched.station_nodes_.size())) {}

  constant_graph_dijkstra<MAX_TRAVEL_TIME, map_station_graph_node> travel_time_;
  constant_graph_dijkstra<MAX_TRANSFERS, map_interchange_graph_node> transfers_;
};

}  // namespace routing
}  // namespace motis
