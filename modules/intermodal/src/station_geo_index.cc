#include "motis/intermodal/station_geo_index.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>

#include "boost/geometry/geometries/point.hpp"
#include "boost/geometry/geometries/box.hpp"
#include "boost/geometry/index/rtree.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

typedef bg::cs::spherical_equatorial<bg::degree> coordinate_system;
typedef bg::model::point<double, 2, coordinate_system> spherical_point;

typedef bg::model::box<spherical_point> box;
typedef std::pair<spherical_point, int> value;

enum { LNG, LAT };

namespace motis {
namespace intermodal {

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
  double r_earth = 6371000.0f;
  return boost::geometry::distance(a, b) * r_earth;
}

struct station_geo_index::impl {
public:
  explicit impl(const std::vector<station_ptr>& stations)
      : stations_(stations) {
    for (const auto& s : stations) {
      auto p = spherical_point(s->length, s->width);
      rtree_.insert(std::make_pair(p, s->index));
    }
  }

  std::vector<const station*> stations(double lat, double lng,
                                       double radius) const {
    std::vector<const station*> stations;

    spherical_point query_point = spherical_point(lng, lat);

    std::vector<value> result_n;
    rtree_.query(bgi::intersects(generate_box(query_point, radius)) &&
                     bgi::satisfies([&query_point, radius](const value& v) {
                       return distance_in_m(v.first, query_point) < radius;
                     }),
                 std::back_inserter(result_n));

    for (const auto& result : result_n) {
      stations.push_back(stations_[result.second].get());
    }

    return stations;
  }

private:
  const std::vector<station_ptr>& stations_;
  bgi::rtree<value, bgi::quadratic<16> > rtree_;
};

station_geo_index::station_geo_index(const std::vector<station_ptr>& stations)
    : impl_(new impl(stations)) {}

station_geo_index::~station_geo_index() {}

std::vector<const station*> station_geo_index::stations(double lat, double lng,
                                                        double radius) const {
  return impl_->stations(lat, lng, radius);
}

}  // namespace intermodal
}  // namespace motis
