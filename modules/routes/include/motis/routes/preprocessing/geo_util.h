#pragma once

#include "boost/geometry.hpp"
#include "boost/geometry/geometries/point.hpp"

#include "motis/routes/preprocessing/osm/osm_node.h"

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

namespace motis {
namespace routes {

typedef bg::cs::spherical_equatorial<bg::degree> coordinate_system;
typedef bg::model::point<double, 2, coordinate_system> spherical_point;
typedef bg::model::box<spherical_point> box;
typedef bg::model::linestring<spherical_point> linestring;

inline box create_bounding_box(linestring const& ls) {
  box bounding_box;
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

inline bool check_distance(double lat1, double lon1, double lat2, double lon2,
                           int max_dist) {
  return distance(spherical_point(lon1, lat1), spherical_point(lon2, lat2)) *
             6378137 <
         max_dist;
}

}  // namespace routes
}  // namespace motis
