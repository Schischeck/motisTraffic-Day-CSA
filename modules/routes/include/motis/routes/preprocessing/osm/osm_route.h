#pragma once

#include <string>
#include <vector>

#include "osmium/osm.hpp"

namespace motis {
namespace routes {

class osm_route {
public:
  explicit osm_route(uint8_t clasz_) : clasz_(clasz_) {}

  std::vector<int64_t> railways_;
  uint8_t clasz_;
};

}  // namespace routes
}  // namespace motis
