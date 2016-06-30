#pragma once

#include <cassert>

#include "boost/geometry/geometries/geometries.hpp"

#include "motis/core/common/geo.h"
#include "motis/routes/preproc/osm/osm_node.h"

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

namespace motis {
namespace routes {

using linestring = bg::model::linestring<geo_detail::spherical_point>;

inline geo_detail::box create_bounding_box(linestring const& ls) {
  geo_detail::box bounding_box;
  envelope(ls, bounding_box);
  return bounding_box;
}

inline linestring create_linestring_from_stations(
    std::vector<uint32_t> const& route, schedule const& sched) {
  linestring ls;
  for (auto const station_id : route) {
    ls.emplace_back(sched.stations_[station_id]->length_,
                    sched.stations_[station_id]->width_);
  }
  return ls;
}

inline linestring create_linestring_from_ways(
    std::vector<int64_t> const& route,
    std::map<int64_t, std::vector<int64_t>> const& id_to_way_,
    std::map<int64_t, osm_node> const& id_to_node_) {
  linestring ls;
  for (auto const way : route) {
    for (auto const node : id_to_way_.at(way)) {
      ls.emplace_back(id_to_node_.at(node).location_.lon(),
                      id_to_node_.at(node).location_.lat());
    }
  }
  return ls;
}

inline bool check_distance(double lat1, double lng1, double lat2, double lng2,
                           int max_dist) {
  return geo_detail::distance_in_m({lng1, lat1}, {lng2, lat2}) < max_dist;
}

inline double get_length(std::vector<coord> const& coords) {
  assert(coords.size() >= 2);
  assert(coords.size() % 2 == 0);

  double res = 0;
  for (unsigned i = 0; i < coords.size() - 1; ++i) {
    res += geo_detail::distance_in_m({coords[i].lat_, coords[i].lng_},
                                     {coords[i + 1].lng_, coords[i + 1].lng_});
  }
  return res;
}

}  // namespace routes
}  // namespace motis