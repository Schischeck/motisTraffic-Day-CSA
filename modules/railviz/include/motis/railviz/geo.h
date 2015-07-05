#pragma once

namespace motis {
namespace railviz {
namespace geo {

struct coord {
  double lat, lng;
};

struct box {
  coord min, max;
};

}  // geo
}  // namespace motis
}  // namespace railviz