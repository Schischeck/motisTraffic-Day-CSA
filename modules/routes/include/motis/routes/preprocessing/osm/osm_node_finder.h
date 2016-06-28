#pragma once

#include "osmium/handler.hpp"
#include "osmium/osm.hpp"

#include "motis/routes/preprocessing/osm/osm_node.h"

namespace motis {
namespace routes {

class osm_node_finder : public osmium::handler::Handler {
public:
  explicit osm_node_finder(std::map<int64_t, osm_node>& id_to_node)
      : id_to_node_(id_to_node) {}

  void node(osmium::Node const& node) {
    auto it = id_to_node_.find(node.id());
    if (it != end(id_to_node_)) {
      it->second.location_ = node.location();
    }
  }

  std::map<int64_t, osm_node>& id_to_node_;
};

}  // namespace routes
}  // namespace motis
