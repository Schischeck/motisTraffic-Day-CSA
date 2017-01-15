#pragma once

#include <string>
#include <vector>

#include "geo/polyline.h"

namespace motis {
namespace path {

struct rail_way {
  rail_way(int64_t from, int64_t to, int64_t id, geo::polyline polyline)
      : from_(from), to_(to), ids_({id}), polyline_(std::move(polyline)) {}

  rail_way(int64_t from, int64_t to, std::vector<int64_t> ids,
           geo::polyline polyline)
      : from_(from),
        to_(to),
        ids_(std::move(ids)),
        polyline_(std::move(polyline)) {}

  bool is_valid() const { return !ids_.empty(); }
  void invalidate() { ids_.clear();}

  int64_t from_, to_;

  std::vector<int64_t> ids_;
  geo::polyline polyline_;
};

std::vector<rail_way> build_rail_ways(std::string const& osm_file);

}  // namespace path
}  // namespace motis
 