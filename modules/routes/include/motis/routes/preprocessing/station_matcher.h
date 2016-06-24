#pragma once

#include <array>
#include <map>
#include <vector>

#include "motis/core/common/logging.h"
#include "motis/core/schedule/schedule.h"
#include "motis/routes/preprocessing/geo_util.h"
#include "motis/routes/preprocessing/node_geo_index.h"
#include "motis/routes/preprocessing/osm/osm_node.h"
#include "motis/routes/preprocessing/osm/osm_route.h"
#include "motis/protocol/RoutesSections_generated.h"

namespace motis {
namespace routes {

class station_matcher {
public:
  station_matcher(std::map<int64_t, osm_node>& osm_nodes,
                  std::map<int64_t, osm_route> const& osm_routes,
                  schedule const& schedule);

  void find_railways(std::string file_name);

  void add_nodes();

  std::vector<int64_t> intersect(uint32_t id1, uint32_t id2, uint8_t clasz);

  std::vector<int64_t> extract_nodes(std::vector<int64_t> const& nodes,
                                     int64_t n1, int64_t n2);

  uint8_t min_clasz(int32_t station_index);

  std::vector<std::tuple<int, uint8_t, std::vector<double>>> find_routes(
      station_node const& station_node);

  std::vector<std::vector<std::pair<int64_t, int64_t>>> station_rels_;
  std::map<int64_t, osm_node>& osm_nodes_;
  std::map<int64_t, osm_route> const& osm_routes_;
  std::vector<flatbuffers::Offset<RoutesSection>> rail_roads_;
  flatbuffers::FlatBufferBuilder builder_;
  schedule const& sched_;
};

}  // namespace routes
}  // namespace motis
