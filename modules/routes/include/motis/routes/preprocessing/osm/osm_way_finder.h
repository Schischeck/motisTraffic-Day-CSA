#pragma once

#include <iostream>
#include <map>
#include <vector>

#include "osmium/handler.hpp"
#include "osmium/osm.hpp"

#include "motis/routes/preprocessing/osm/osm_node.h"

namespace motis {
namespace routes {

class osm_way_finder : public osmium::handler::Handler {
public:
  osm_way_finder(std::map<int64_t, std::vector<int64_t>>& id_to_way,
                 std::map<int64_t, osm_node>& id_to_node)
      : id_to_node_(id_to_node), id_to_way_(id_to_way) {}

  void way(osmium::Way const& way) {
    if (!id_to_way_.count(way.id())) {
      return;
    }

    for (auto const& node : way.nodes()) {
      id_to_way_.at(way.id()).push_back(node.ref());
      if (id_to_node_.count(node.ref()) == 0) {
        id_to_node_.emplace(node.ref(), osm_node(node.ref()));
      }
    }
  }

  std::map<int64_t, osm_node>& id_to_node_;
  std::map<int64_t, std::vector<int64_t>>& id_to_way_;
};

}  // namespace routes
}  // namespace motis
