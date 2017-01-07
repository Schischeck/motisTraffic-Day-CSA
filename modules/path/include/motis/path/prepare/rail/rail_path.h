#pragma once

#include <vector>

#include "motis/path/prepare/rail/rail_graph.h"
#include "motis/path/prepare/rail/rail_phantom.h"

namespace motis {
namespace path {

struct rail_path {
  std::vector<rail_edge const*> edges_;
  rail_phantom const* source_;
  rail_phantom const* goal_;
  bool valid_;
};

double length(rail_graph const&, rail_path const&);

geo::polyline to_polyline(rail_graph const&, rail_path const&);

}  // namespace path
}  // namespace motis
