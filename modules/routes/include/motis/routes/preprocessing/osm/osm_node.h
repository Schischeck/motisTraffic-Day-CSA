#pragma once

#include <string>
#include <vector>

#include "osmium/osm.hpp"

namespace motis {
namespace routes {

class osm_node {
public:
  explicit osm_node(int64_t id) : id_(id) {}

  int64_t id_;
  std::vector<int64_t> relations_;
  osmium::Location location_;
};

}  // namespace routes
}  // namespace motis
