#pragma once

#include <vector>

#include "boost/function_output_iterator.hpp"
#include "boost/geometry/geometries/box.hpp"
#include "boost/geometry/geometries/point.hpp"
#include "boost/geometry/index/rtree.hpp"

#include "motis/core/common/transform_to_vec.h"

// TODO refactor this to use geo::latlng

namespace motis {
namespace routes {

struct point_rtree {
  using cs = boost::geometry::cs::spherical_equatorial<boost::geometry::degree>;

  using point = boost::geometry::model::point<double, 2, cs>;
  enum { LNG, LAT };

  using box = boost::geometry::model::box<point>;
  using value = std::pair<point, size_t>;

  using rtree_t =
      boost::geometry::index::rtree<value,
                                    boost::geometry::index::quadratic<16>>;

  explicit point_rtree(rtree_t rtree) : rtree_(std::move(rtree)) {}

  std::vector<std::pair<double, size_t>> in_radius_with_distance(
      double const lat, double const lng, double const min_radius,
      double const max_radius) const {

    auto const query_point = point{lng, lat};

    std::vector<std::pair<double, size_t>> results;
    rtree_.query(boost::geometry::index::intersects(
                     generate_box(query_point, max_radius)),
                 boost::make_function_output_iterator([&](auto&& v) {
                   auto const distance = distance_in_m(v.first, query_point);
                   if (distance >= max_radius || distance < min_radius) {
                     return;
                   }
                   results.emplace_back(distance, v.second);
                 }));

    std::sort(begin(results), end(results));
    return results;
  }

  std::vector<std::pair<double, size_t>> in_radius_with_distance(
      double const lat, double const lng, double const max_radius) const {
    return in_radius_with_distance(lat, lng, 0, max_radius);
  }

  std::vector<size_t> in_radius(double const lat, double const lng,
                                double const min_radius,
                                double const max_radius) const {
    return transform_to_vec(
        in_radius_with_distance(lat, lng, min_radius, max_radius),
        [](auto&& r) { return r.second; });
  }

  std::vector<size_t> in_radius(double const lat, double const lng,
                                double const max_radius) const {
    return in_radius(lat, lng, 0, max_radius);
  }

private:
  /// Generates a query box around the given origin with edge length 2xdist
  static inline box generate_box(point const& center, double dist_in_m) {
    // The distance of latitude degrees in km is always the same (~111000.0f)
    double d_lat = dist_in_m / 111000.0f;

    // The distance of longitude degrees depends on the latitude.
    double origin_lat_rad = center.get<LAT>() * (M_PI / 180.0f);
    double m_per_deg = 111200.0f * std::cos(origin_lat_rad);
    double d_lng = std::abs(dist_in_m / m_per_deg);

    return box(point(center.get<LNG>() - d_lng, center.get<LAT>() - d_lat),
               point(center.get<LNG>() + d_lng, center.get<LAT>() + d_lat));
  }

  /// Computes the distance (in meters) between two coordinates
  static inline double distance_in_m(point const& a, point const& b) {
    constexpr double earth_radius_meters = 6371000.0f;
    return boost::geometry::distance(a, b) * earth_radius_meters;
  }

  rtree_t rtree_;
};

template <typename C, typename F>
point_rtree make_point_rtree(C const& container, F fun) {
  // fun(e) should return a point: {e.lng, e.lat}
  auto i = 0;
  return point_rtree{point_rtree::rtree_t{
      transform_to_vec(container, [&](auto&& e) -> point_rtree::value {
        return {fun(e), i++};
      })}};
}

}  // namespace routes
}  // namespace motis
