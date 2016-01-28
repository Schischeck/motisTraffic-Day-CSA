#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>

#include "boost/geometry/geometries/point.hpp"
#include "boost/geometry/geometries/box.hpp"
#include "boost/geometry/index/rtree.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace motis {

namespace geo_detail {

constexpr double kEarthRadiusMeters = 6371000.0f;

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

using coordinate_system = bg::cs::spherical_equatorial<bg::degree>;
using spherical_point = bg::model::point<double, 2, coordinate_system>;
using box = bg::model::box<spherical_point>;
using value = std::pair<spherical_point, size_t>;

using quadratic_rtree = bgi::rtree<value, bgi::quadratic<16>>;

enum { LNG, LAT };

/// Generates a query box around the given origin with edge length 2xdist
box generate_box(const spherical_point& center, double dist_in_m) {
  // The distance of latitude degrees in km is always the same (~111000.0f)
  double lat_diff = dist_in_m / 111000.0f;

  // The distance of longitude degrees depends on the latitude.
  double origin_lat_rad = center.get<LAT>() * (M_PI / 180.0f);
  double m_per_deg = 111200.0f * std::cos(origin_lat_rad);
  double lng_diff = std::abs(dist_in_m / m_per_deg);

  auto top_left = spherical_point(bg::get<LNG>(center) + lng_diff,
                                  bg::get<LAT>(center) + lat_diff);
  auto bottom_right = spherical_point(bg::get<LNG>(center) - lng_diff,
                                      bg::get<LAT>(center) - lat_diff);

  return box(bottom_right, top_left);
}

/// Computes the distance (in meters) between two coordinates
double distance_in_m(const spherical_point& a, const spherical_point& b) {
  return boost::geometry::distance(a, b) * kEarthRadiusMeters;
}

inline double distance_in_m(double a_lat, double a_lng, double b_lat,
                            double b_lng) {
  return geo_detail::distance_in_m({a_lng, a_lat}, {b_lng, b_lat});
}

}  // namespace geo_detail
}  // namespace motis
