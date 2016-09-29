#pragma once

#include "motis/geo/polyline.h"

#include "motis/routes/prepare/rel/relation_parser.h"

namespace motis {
namespace routes {

struct aggregated_polyline {
  aggregated_polyline(relation const* r, polyline p)
      : relation_(relation), polyline_(std::move(p)) {}

  relation const* relation_;
  polyline polyline_;
};

std::vector<aggregated_polyline> aggregate_polylines(std::vector<relation>&);

}  // namespace routes
}  // namespace motis
