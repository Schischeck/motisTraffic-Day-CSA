#pragma once

#include <map>
#include <vector>

#include "motis/core/schedule/schedule.h"

namespace motis {
namespace railviz {

struct path_resolver {
  path_resolver(schedule const& sched, int zoom_level);

  std::vector<std::vector<double>> get_trip_path(trip const* trp);
  std::vector<double> get_segment_path(edge const* e);

private:
  schedule const& sched_;
  unsigned zoom_level_;
  std::map<std::vector<trip::route_edge> const*,
           std::vector<std::vector<double>>>
      trip_cache_;
  std::map<edge const*, std::vector<double>> edge_cache_;
};

}  // namespace railviz
}  // namespace motis
