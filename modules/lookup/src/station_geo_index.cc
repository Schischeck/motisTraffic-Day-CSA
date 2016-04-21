#include "motis/lookup/station_geo_index.h"

#include "motis/core/common/geo.h"

using namespace motis::geo_detail;
using namespace flatbuffers;

namespace motis {
namespace lookup {

struct coordinate {
  double lat, lng;
};

template <typename T>
constexpr T identity(T&& v) {
  return std::forward<T>(v);
}

struct station_geo_index::impl {
public:
  explicit impl(std::vector<station_ptr> const& stations)
      : stations_(stations) {
    std::vector<value> values;
    for (size_t i = 0; i < stations.size(); ++i) {
      values.push_back(std::make_pair(
          spherical_point(stations[i]->length, stations[i]->width), i));
    }
    rtree_ = quadratic_rtree{values};
  }

  std::vector<const station*> stations(double lat, double lng,
                                       double radius) const {
    std::vector<station const*> vec;
    spherical_point query_point = spherical_point(lng, lat);

    std::vector<value> result_n;
    rtree_.query(bgi::intersects(generate_box(query_point, radius)) &&
                     bgi::satisfies([&query_point, radius](const value& v) {
                       return distance_in_m(v.first, query_point) < radius;
                     }),
                 std::back_inserter(result_n));

    for (const auto& result : result_n) {
      vec.push_back(stations_[result.second].get());
    }

    return vec;
  }

  flatbuffers::Offset<LookupGeoStationResponse> stations(
      FlatBufferBuilder& fbb, LookupGeoStationRequest const* req) const {
    std::vector<Offset<Station>> list;
    for (auto const& station : stations(req->lat(), req->lng(), req->radius())) {
      list.push_back(CreateStation(fbb, fbb.CreateString(station->eva_nr),
                                   fbb.CreateString(station->name),
                                   station->lat(), station->lng()));
    }
    return CreateLookupGeoStationResponse(fbb, fbb.CreateVector(list));
  }

private:
  std::vector<station_ptr> const& stations_;
  quadratic_rtree rtree_;
};

station_geo_index::station_geo_index(const std::vector<station_ptr>& stations)
    : impl_(new impl(stations)) {}

station_geo_index::~station_geo_index() {}

std::vector<const station*> station_geo_index::stations(double lat, double lng,
                                                        double radius) const {
  return impl_->stations(lat, lng, radius);
}

flatbuffers::Offset<LookupGeoStationResponse> station_geo_index::stations(
    FlatBufferBuilder& fbb, LookupGeoStationRequest const* req) const {
  return impl_->stations(fbb, req);
}

}  // namespace lookup
}  // namespace motis
