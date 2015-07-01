#pragma once

#include <memory>
#include <vector>

#include "motis/railviz/geometry.h"

namespace motis {
struct edge;
struct schedule;
}

namespace motis {
namespace railviz {

class edge_geo_index {
public:
  explicit edge_geo_index(schedule const&);
  virtual ~edge_geo_index();

  std::vector<edge const*> edges(double bottom_right_lat,
                                 double bottom_right_lng, double top_left_lat,
                                 double top_left_lng) const;
  geometry::box get_bounds() const;

private:
  class impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace railviz
}  // namespace motis
