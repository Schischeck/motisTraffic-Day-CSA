#pragma once

#include "motis/core/schedule/schedule.h"
#include "motis/routes/preproc/db/railway_graph.h"

namespace motis {
namespace routes {

struct railway_finder {
public:
  railway_finder(schedule const& sched, railway_graph& graph);

  void find_railways(std::string file);

private:
  // std::vector<double> shortest_path(station_node const& start,
  //                                   station_node const& end);
  // std::vector<int> get_connected_stations(station_node const& node);

  schedule const& sched_;
  railway_graph& graph_;
};

}  // namespace routes
}  // namespace motis
