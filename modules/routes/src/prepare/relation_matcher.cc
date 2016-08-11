#include "motis/routes/prepare/relation_matcher.h"

#include <limits>

#include "motis/routes/prepare/point_rtree.h"

namespace motis {
namespace routes {

relation_matcher::relation_matcher(motis::loader::Schedule const* sched,
                                   std::string const& osm_file)
    : sched_(sched), osm_file_(osm_file) {
  load_osm();
};

void relation_matcher::load_osm() {
  foreach_osm_relation(osm_file_, [&](auto&& relation) {
    std::string const type = relation.get_value_by_key("type", "");
    std::string const route = relation.get_value_by_key("route", "");
    if ((type == "route" || type == "public_transport") &&
        (route == "subway" || route == "light_rail" || route == "railway" ||
         route == "train" || route == "tram" || route == "bus")) {
      relations_.emplace(relation.id(), osm_relation(relation.id()));
      std::vector<int64_t> ways;
      for (auto const& member : relation.members()) {
        if (member.type() == osmium::item_type::way) {
          auto way =
              way_to_relations_.emplace(member.ref(), std::vector<int64_t>());
          way.first->second.push_back(relation.id());
        }
      }
    }
  });
  foreach_osm_way(osm_file_, [&](auto&& way) {
    auto r = way_to_relations_.find(way.id());
    if (r == way_to_relations_.end()) {
      return;
    }
    for (auto const& node : way.nodes()) {
      for (auto relation : r->second) {
        auto rel = relations_.find(relation);
        if (rel != relations_.end()) {
          rel->second.nodes_.push_back(node.ref());
        }
      }
      nodes_.emplace(node.ref(), osm_node(node.ref()));
    }
  });
  foreach_osm_node(osm_file_, [&](auto&& node) {
    auto n = nodes_.find(node.id());
    if (n != std::end(nodes_)) {
      n->second.location_ = node.location();
    }
  });
  std::cout << "Finished extracting" << std::endl;
  std::cout << "Nodes: " << nodes_.size() << std::endl;
  std::cout << "Relations: " << relations_.size() << std::endl;
}

void relation_matcher::find_perfect_matches() {
  std::cout << "Finding Perfect Matches" << std::endl;
  std::cout << "Routes: " << sched_->routes()->size() << std::endl;
  int count = 0;
  int result = 0;
  std::vector<int> matched_routes;
  for (auto const& rel : relations_) {
    std::vector<osmium::Location> locs;
    for (auto const& r : rel.second.nodes_) {
      locs.push_back(nodes_.at(r).location_);
    }
    auto rtree = make_point_rtree(locs, [](auto&& s) {
      return point_rtree::point{s.lon(), s.lat()};
    });
    auto route_count = 0;
    for (auto const& r : *sched_->routes()) {
      std::cout << count << "/" << relations_.size() << "|" << route_count
                << "/" << sched_->routes()->size() << "|   " << result << "\r";
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
