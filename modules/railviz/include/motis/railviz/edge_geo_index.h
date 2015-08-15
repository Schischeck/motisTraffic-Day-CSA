#pragma once

#include <memory>
#include <vector>

#include "motis/railviz/geo.h"

namespace motis {
struct edge;
struct schedule;
}

namespace motis {
namespace railviz {

class edge_geo_index {
 public:
  explicit edge_geo_index(int clasz, schedule const&);
  virtual ~edge_geo_index();

  std::vector<edge const*> edges(geo::box area) const;
  geo::box get_bounds() const;

 private:
  class impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace railviz
}  // namespace motis
