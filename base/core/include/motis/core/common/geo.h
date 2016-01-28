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

template <typename V>
struct geo_index {

  geo_index(std::vector<std::unique_ptr<V>> const& entries,
            std::vector<value> const& values)
      : entries_(entries), rtree_(values) {}

  std::vector<V const*> in_radius(double lat, double lng, double radius) const {
    std::vector<V const*> vec;
    spherical_point query_point = spherical_point(lng, lat);

    std::vector<value> result_n;
    rtree_.query(bgi::intersects(generate_box(query_point, radius)) &&
                     bgi::satisfies([&query_point, radius](const value& v) {
                       return distance_in_m(v.first, query_point) < radius;
                     }),
                 std::back_inserter(result_n));

    for (const auto& result : result_n) {
      vec.push_back(entries_[result.second].get());
    }

    return vec;
  }

  std::vector<std::unique_ptr<V>> const& entries_;
  bgi::rtree<value, bgi::quadratic<16>> rtree_;
};

template <typename T, typename F>
geo_index<T> make_geo_index(std::vector<std::unique_ptr<T>> const& entries,
                            F func) {
  std::vector<value> values;
  for (size_t i = 0; i < entries.size(); ++i) {
    std::pair<double, double> latlng = func(entries[i]);
    values.push_back({{latlng.second, latlng.first}, i});
  }
  return {entries, values};
}
}  // namespace geo_detail

using geo_detail::geo_index;
using geo_detail::make_geo_index;

inline double distance_in_m(double a_lat, double a_lng, double b_lat,
                            double b_lng) {
  return geo_detail::distance_in_m({a_lng, a_lat}, {b_lng, b_lat});
}

}  // namespace motis
