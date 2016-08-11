#pragma once

#include <string>
#include <vector>

#include "osmium/osm.hpp"

namespace motis {
namespace routes {

class osm_relation {
public:
  osm_relation(int64_t id) : id_(id) {}

  std::vector<int64_t> nodes_;
  int64_t id_;
};

}  // namespace routes
}  // namespace motis
