#pragma once

#include <map>
#include <memory>
#include <vector>

#include "motis/core/schedule/station.h"
#include "motis/routes/preproc/osm/osm_node.h"

namespace motis {
namespace routes {

class node_geo_index {
public:
  explicit node_geo_index(std::map<int64_t, osm_node> const& osm_nodes);
  ~node_geo_index();

  std::vector<int64_t> nodes(double lat, double lng, double radius) const;

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace routes
}  // namespace motis
