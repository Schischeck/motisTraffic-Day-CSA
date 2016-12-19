#pragma once

#include "geo/polyline.h"

#include "motis/path/prepare/rel/relation_parser.h"

namespace motis {
namespace path {

struct aggregated_polyline {
  aggregated_polyline(source_spec source, geo::polyline p)
      : source_(source), polyline_(std::move(p)) {}

  source_spec source_;
  geo::polyline polyline_;
};

std::vector<aggregated_polyline> aggregate_polylines(
    std::vector<relation> const&);

}  // namespace path
}  // namespace motis
