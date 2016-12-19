#pragma once

namespace motis {
namespace railviz {
namespace geo {

struct coord {
  double lat_, lng_;
};

using box = std::pair<coord, coord>;

}  // namespace geo
}  // namespace railviz
}  // namespace motis
