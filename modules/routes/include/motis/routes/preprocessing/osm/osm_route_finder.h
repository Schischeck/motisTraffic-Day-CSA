#pragma once

#include <iostream>

#include "osmium/handler.hpp"
#include "osmium/osm.hpp"

#include "motis/routes/preprocessing/osm/osm_node.h"
#include "motis/routes/preprocessing/osm/osm_route.h"

namespace motis {
namespace routes {

class osm_route_finder : public osmium::handler::Handler {
public:
  osm_route_finder(std::map<int64_t, std::vector<int64_t>>& id_to_way,
                   std::map<int64_t, osm_route>& routes,
                   std::map<int64_t, std::vector<int64_t>>& relation_to_ways)
      : id_to_way_(id_to_way),
        routes_(routes),
        relation_to_ways_(relation_to_ways) {}

  void relation(osmium::Relation const& relation) {
    auto const type = relation.tags()["type"];
    auto const route = relation.tags()["route"];
    if (type && route && is_route_tag(route, type)) {
      routes_.emplace(relation.id(),
                      osm_route(get_clasz(route, relation.tags()["ref"])));
      std::vector<int64_t> ways;
      for (auto const& member : relation.members()) {
        if (member.type() == osmium::item_type::way) {
          ways.push_back(member.ref());
          id_to_way_.emplace(member.ref(), std::vector<int64_t>());
        }
      }
      relation_to_ways_.emplace(relation.id(), ways);
    }
  }

  bool is_route_tag(char const* tag, char const* type) {
    std::string route_tag(tag);
    std::string relation_type(type);
    return relation_type == "route" &&
           (route_tag == "subway" || route_tag == "light_rail" ||
            route_tag == "railway" || route_tag == "train" ||
            route_tag == "tram");
  }

  uint8_t get_clasz(char const* tag, char const* ref) {
    std::string route_tag(tag);
    if (route_tag == "subway") {
      return 6;
    } else if (route_tag == "tram") {
      return 7;
    } else if (route_tag == "light_rail") {
      return 5;
    } else if ((route_tag == "train" || route_tag == "railway") && ref) {
      std::string ref_tag(ref);
      if (ref_tag.find("RB") != std::string::npos) {
        return 4;
      }
      if (ref_tag.find("RE") != std::string::npos) {
        return 3;
      }
      if (ref_tag.find("ICE") != std::string::npos) {
        return 0;
      }
      if (ref_tag.find("IC") != std::string::npos) {
        return 1;
      }
      if (ref_tag.find("N") != std::string::npos) {
        return 2;
      }
    }
    return 9;
  }

  std::map<int64_t, std::vector<int64_t>>& id_to_way_;
  std::map<int64_t, osm_route>& routes_;
  std::map<int64_t, std::vector<int64_t>>& relation_to_ways_;
};

}  // namespace routes
}  // namespace motis
