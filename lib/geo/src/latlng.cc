#include "motis/geo/latlng.h"

#include "boost/geometry.hpp"

#include "motis/geo/constants.h"
#include "motis/geo/detail/register.h"

namespace motis {
namespace geo {

double distance(latlng const& a, latlng const& b) {
  return boost::geometry::distance(a, b) * kEarthRadiusMeters;
}

}  // namespace geo
}  // namespace motis
