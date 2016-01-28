#include "motis/intermodal/station_geo_index.h"

#include "motis/core/common/geo.h"

namespace motis {
namespace intermodal {

struct station_geo_index::impl {
public:
  explicit impl(const std::vector<station_ptr>& stations)
      : index_(make_geo_index(
            stations, [](station_ptr const& s) -> std::pair<double, double> {
              return {s->width, s->length};
            })) {}

  std::vector<const station*> stations(double lat, double lng,
                                       double radius) const {
    return index_.in_radius(lat, lng, radius);
  }

private:
  geo_index<station> index_;
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
