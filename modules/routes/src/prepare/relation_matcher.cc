#include "motis/routes/prepare/relation_matcher.h"

#include <limits>
#include <map>
#include <vector>

#include "osmium/handler.hpp"
#include "osmium/io/pbf_input.hpp"
#include "osmium/io/reader_iterator.hpp"
#include "osmium/io/xml_input.hpp"
#include "osmium/memory/buffer.hpp"
#include "osmium/osm.hpp"
#include "osmium/visitor.hpp"

#include "motis/routes/prepare/osm_node.h"
#include "motis/routes/prepare/osm_relation.h"
#include "motis/routes/prepare/point_rtree.h"

namespace motis {
namespace routes {

template <typename F>
void foreach_osm_node(std::string const& filename, F f) {
  osmium::io::Reader reader(filename, osmium::osm_entity_bits::node);
  for (auto it = std::begin(reader); it != std::end(reader); ++it) {
    f(static_cast<osmium::Node&>(*it));
  }
}

template <typename F>
void foreach_osm_way(std::string const& filename, F f) {
  osmium::io::Reader reader(filename, osmium::osm_entity_bits::way);
  for (auto it = std::begin(reader); it != std::end(reader); ++it) {
    f(static_cast<osmium::Way&>(*it));
  }
}

template <typename F>
void foreach_osm_relation(std::string const& filename, F f) {
  osmium::io::Reader reader(filename, osmium::osm_entity_bits::relation);
  for (auto it = std::begin(reader); it != std::end(reader); ++it) {
    f(static_cast<osmium::Relation&>(*it));
  }
}

void find_perfect_matches(motis::loader::Schedule const* sched,
                          std::string const& osm_file) {
  std::map<int64_t, osm_node> nodes;
  std::map<int64_t, osm_relation> relations;
  std::map<int64_t, std::vector<int64_t>> way_to_relations;

  foreach_osm_relation(osm_file, [&](auto&& relation) {
    std::string const type = relation.get_value_by_key("type", "");
    std::string const route = relation.get_value_by_key("route", "");
    if ((type == "route" || type == "public_transport") &&
        (route == "subway" || route == "light_rail" || route == "railway" ||
         route == "train" || route == "tram" || route == "bus")) {
      relations.emplace(relation.id(), osm_relation(relation.id()));
      std::vector<int64_t> ways;
      for (auto const& member : relation.members()) {
        if (member.type() == osmium::item_type::way) {
          auto way =
              way_to_relations.emplace(member.ref(), std::vector<int64_t>());
          way.first->second.push_back(relation.id());
        }
      }
    }
  });
  foreach_osm_way(osm_file, [&](auto&& way) {
    auto r = way_to_relations.find(way.id());
    if (r == way_to_relations.end()) {
      return;
    }
    for (auto const& node : way.nodes()) {
      for (auto relation : r->second) {
        auto rel = relations.find(relation);
        if (rel != relations.end()) {
          rel->second.nodes_.push_back(node.ref());
        }
      }
      nodes.emplace(node.ref(), osm_node(node.ref()));
    }
  });
  foreach_osm_node(osm_file, [&](auto&& node) {
    auto n = nodes.find(node.id());
    if (n != std::end(nodes)) {
      n->second.location_ = node.location();
    }
  });
  std::cout << "Finished extracting" << std::endl;
  std::cout << "Nodes: " << nodes.size() << std::endl;
  std::cout << "Relations: " << relations.size() << std::endl;
  std::cout << "Routes: " << sched->routes()->size() << std::endl;

  int count = 0;
  int result = 0;
  std::vector<int> matched_routes;
  for (auto const& rel : relations) {
    std::vector<osmium::Location> locs;
    for (auto const& r : rel.second.nodes_) {
      locs.push_back(nodes.at(r).location_);
    }
    auto rtree = make_point_rtree(locs, [](auto&& s) {
      return point_rtree::point{s.lon(), s.lat()};
    });
    auto route_count = 0;
    for (auto const& r : *sched->routes()) {
      std::cout << count << "/" << relations.size() << "|" << route_count << "/"
                << sched->routes()->size() << "|   " << result << "\r";
      if (std::find(std::begin(matched_routes), std::end(matched_routes),
                    route_count) != std::end(matched_routes)) {
        route_count++;
        continue;
      }
      bool matched = true;
      for (auto const& s : *r->stations()) {
        if (rtree.in_radius(s->lat(), s->lng(), 100).empty()) {
          matched = false;
          break;
        }
      }
      if (matched) {
        matched_routes.push_back(route_count);
        result++;
      }
      route_count++;
    }
    count++;
  }
  std::cout << std::endl;
  std::cout << "Matched routes: " << result << std::endl;
}

}  // namespace routes
}  // namespace motis
