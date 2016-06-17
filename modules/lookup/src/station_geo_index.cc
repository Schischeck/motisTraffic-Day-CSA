#include "motis/lookup/station_geo_index.h"

#include "boost/function_output_iterator.hpp"

#include "motis/core/common/geo.h"
#include "motis/loader/util.h"

using namespace motis::geo_detail;
using namespace motis::loader;
using namespace flatbuffers;

namespace motis {
namespace lookup {

struct station_geo_index::impl {
public:
  explicit impl(std::vector<station_ptr> const& stations)
      : stations_(stations) {
    std::vector<value> values;
    for (size_t i = 0; i < stations.size(); ++i) {
      values.push_back(std::make_pair(
          spherical_point(stations[i]->length_, stations[i]->width_), i));
    }
    rtree_ = quadratic_rtree{values};
  }

  std::vector<station const*> stations(double lat, double lng,
                                       double min_radius,
                                       double max_radius) const {
    spherical_point query_point = spherical_point(lng, lat);

    std::vector<std::pair<double, size_t>> results;
    rtree_.query(bgi::intersects(generate_box(query_point, max_radius)),
                 boost::make_function_output_iterator([&](auto&& v) {
                   auto const distance = distance_in_m(v.first, query_point);
                   if (distance >= max_radius || distance < min_radius) {
                     return;
                   }
                   results.emplace_back(distance, v.second);
                 }));

    std::sort(begin(results), end(results));

    return transform_to_vec(results, [this](auto&& r) -> station const* {
      return stations_[r.second].get();
    });
  }

private:
  std::vector<station_ptr> const& stations_;
  quadratic_rtree rtree_;
};

station_geo_index::station_geo_index(std::vector<station_ptr> const& stations)
    : impl_(new impl(stations)) {}

station_geo_index::~station_geo_index() = default;

std::vector<station const*> station_geo_index::stations(
    double lat, double lng, double min_radius, double max_radius) const {
  return impl_->stations(lat, lng, min_radius, max_radius);
}

}  // namespace lookup
}  // namespace motis
