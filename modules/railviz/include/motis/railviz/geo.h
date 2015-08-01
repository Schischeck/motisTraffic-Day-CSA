#pragma once

#include <algorithm>

namespace motis {
namespace railviz {
namespace geo {

struct coord {
  double lat, lng;
};

struct box {
  coord first, second;
  coord max() const {
    return coord{std::max(first.lat, second.lat),
                 std::max(first.lng, second.lng)};
  }
  coord min() const {
    return coord{std::min(first.lat, second.lat),
                 std::min(first.lng, second.lng)};
  }
};

}  // geo
}  // namespace motis
}  // namespace railviz
