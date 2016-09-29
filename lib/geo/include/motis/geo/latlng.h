#pragma once

#include <tuple>

namespace motis {
namespace geo {

struct latlng {
  latlng() = default;
  latlng(double lat, double lng) : lat_(lat), lng_(lng) {}

  friend bool operator<(latlng const& lhs, latlng const& rhs) {
    return std::tie(lhs.lat_, lhs.lng_) < std::tie(rhs.lat_, rhs.lng_);
  }

  friend bool operator==(latlng const& lhs, latlng const& rhs) {
    return std::tie(lhs.lat_, lhs.lng_) == std::tie(rhs.lat_, rhs.lng_);
  }

  double lat_, lng_;
};

double distance(latlng const&, latlng const&);

double bearing(latlng const&, latlng const&);

}  // namespace geo
}  // namespace motis
