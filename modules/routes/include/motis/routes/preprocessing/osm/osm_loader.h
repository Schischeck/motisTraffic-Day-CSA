#pragma once

#include "osmium/handler.hpp"
#include "osmium/io/pbf_input.hpp"
#include "osmium/io/xml_input.hpp"
#include "osmium/memory/buffer.hpp"
#include "osmium/osm.hpp"
#include "osmium/visitor.hpp"

#include "motis/core/common/logging.h"
#include "motis/routes/preprocessing/osm/osm_node.h"
#include "motis/routes/preprocessing/osm/osm_node_finder.h"
#include "motis/routes/preprocessing/osm/osm_route.h"
#include "motis/routes/preprocessing/osm/osm_route_finder.h"
#include "motis/routes/preprocessing/osm/osm_way_finder.h"

namespace motis {
namespace routes {

template <typename H>
void apply(osmium::io::File const& pbf, H& handler) {
  osmium::io::Reader reader(pbf);
  osmium::apply(reader, handler);
}

class osm_loader {
public:
  osm_loader(std::string osm_file, std::map<int64_t, osm_node>& osm_nodes,
             std::map<int64_t, osm_route>& osm_routes)
      : osm_file_(std::move(osm_file)),
        osm_nodes_(osm_nodes),
        osm_routes_(osm_routes) {}

  void load_osm() {
    motis::logging::scoped_timer timer("loading osm data");
    std::map<int64_t, std::vector<int64_t>> id_to_way;
    std::map<int64_t, std::vector<int64_t>> relation_to_ways;
    osm_route_finder route_finder(id_to_way, osm_routes_, relation_to_ways);
    osm_way_finder way_finder(id_to_way, osm_nodes_);
    osm_node_finder node_finder(osm_nodes_);
    osmium::io::File input_file(osm_file_);
    apply(input_file, route_finder);
    apply(input_file, way_finder);
    apply(input_file, node_finder);
    for (auto const& r : relation_to_ways) {
      for (auto way_id : r.second) {
        for (auto node_id : id_to_way.at(way_id)) {
          osm_routes_.at(r.first).railways_.push_back(node_id);
          osm_nodes_.at(node_id).relations_.push_back(r.first);
        }
      }
    }
  }

  std::string osm_file_;
  std::map<int64_t, osm_node>& osm_nodes_;
  std::map<int64_t, osm_route>& osm_routes_;
};
}  // namespace routes
}  // namespace motis
