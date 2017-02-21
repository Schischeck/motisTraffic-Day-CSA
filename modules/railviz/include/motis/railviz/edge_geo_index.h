#pragma once

#include <memory>
#include <vector>

#include "motis/core/common/hash_map.h"

#include "motis/railviz/geo.h"

namespace motis {
class edge;
struct schedule;
}  // namespace motis

namespace motis {
namespace railviz {

struct edge_geo_index {
public:
  edge_geo_index(int clasz, schedule const&,
                 hash_map<std::pair<int, int>, geo::box> const&);
  virtual ~edge_geo_index();

  std::vector<edge const*> edges(geo::box area) const;
  geo::box get_bounds() const;

private:
  class impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace railviz
}  // namespace motis
