#include "parser/file.h"

#include "motis/routes/preprocessing/station_matcher.h"

using namespace motis::logging;

namespace motis {
namespace routes {

station_matcher::station_matcher(std::map<int64_t, osm_node>& osm_nodes,
                                 std::map<int64_t, osm_route> const& osm_routes,
                                 schedule const& schedule)
    : station_rels_(schedule.stations_.size()),
      osm_nodes_(osm_nodes),
      osm_routes_(osm_routes),
      sched_(schedule) {
  scoped_timer timer("generating geo index");
  node_geo_index node_index(osm_nodes_);
  for (auto const& station : sched_.stations_) {
    auto& rels = station_rels_[station->index_];
    for (auto const& id :
         node_index.nodes(station->width_, station->length_, 500)) {
      auto const it = osm_nodes.find(id);
      if (it == end(osm_nodes)) {
        continue;
      }
      for (auto const& rel : it->second.relations_) {
        if (std::find_if(begin(rels), end(rels), [&rel](auto&& a) {
              return a.first == rel;
            }) == rels.end()) {
          rels.emplace_back(rel, id);
        }
      }
    }
  }
}

void station_matcher::find_railways(postgres_writer& writer) {
  scoped_timer timer("finding railway nodes");
  writer.add_table("railway_nodes", "(clasz integer, way geometry)",
                   "(clasz, way)", 2);
  for (auto const& station_node : sched_.station_nodes_) {
    auto results = find_routes(*station_node);
    for (auto const& nodes : results) {
      writer.add_entry({std::to_string(std::get<1>(nodes)),
                        writer.make_line(std::get<2>(nodes))});
      rail_roads_.push_back(CreateRoutesSection(
          builder_, station_node->id_, std::get<0>(nodes), std::get<1>(nodes),
          builder_.CreateVector(std::get<2>(nodes))));
    }
  }
  writer.end_table();
  auto railroads_fb =
      CreateRoutesSections(builder_, builder_.CreateVector(rail_roads_));
  builder_.Finish(railroads_fb);
  parser::file("test.raw", "w+")
      .write(builder_.GetBufferPointer(), builder_.GetSize());
};

std::vector<std::tuple<int, uint8_t, std::vector<double>>>
station_matcher::find_routes(station_node const& station_node) {
  std::vector<std::pair<unsigned int, uint8_t>> visited;
  std::vector<std::tuple<int, uint8_t, std::vector<double>>> results;
  for (auto const& route_node : station_node.get_route_nodes()) {
    for (auto const& edge : route_node->edges_) {
      if (edge.empty()) {
        continue;
      }
      auto const& dest = edge.get_destination()->get_station();
      auto const& clasz = edge.m_.route_edge_.conns_[0].full_con_->clasz_;
      std::vector<double> route;
      if (clasz == 8 ||
          std::find_if(
              visited.begin(), visited.end(),
              [dest, clasz](std::pair<unsigned int, uint8_t> a) -> bool {
                return a.first == dest->id_ && a.second == clasz;
              }) != visited.end()) {
        continue;
      }
      auto nodes = intersect(station_node.id_, dest->id_, clasz);
      if (nodes.empty()) {
        auto const& station1 = sched_.stations_[station_node.id_];
        auto const& station2 = sched_.stations_[dest->id_];
        route.push_back(station1->width_);
        route.push_back(station1->length_);
        route.push_back(station2->width_);
        route.push_back(station2->length_);
      } else {
        for (auto const& id : nodes) {
          auto const& n = osm_nodes_.at(id);
          route.push_back(n.location_.lat());
          route.push_back(n.location_.lon());
        }
      }
      if (!route.empty()) {
        results.push_back(std::make_tuple(dest->id_, clasz, route));
      }
      visited.push_back(std::make_pair(dest->id_, clasz));
    }
  }
  std::sort(results.begin(), results.end(),
            [](auto const& lhs, auto const& rhs) {
              return std::tie(std::get<0>(lhs), std::get<1>(lhs)) <
                     std::tie(std::get<0>(rhs), std::get<1>(rhs));
            });
  return results;
};

void station_matcher::export_stations(postgres_writer& writer) {
  scoped_timer timer("exporting stations");
  writer.add_table("motis_station",
                   "(name TEXT NOT NULL, clasz integer, way geometry)",
                   "(name, clasz, way)", 3);
  for (auto const& station : sched_.stations_) {
    auto clasz = min_clasz(station->index_);
    writer.add_entry({station->name_, std::to_string(clasz),
                      writer.make_point(station->length_, station->width_)});
  }
  writer.end_table();
}

uint8_t station_matcher::min_clasz(int32_t station_index) {
  uint8_t min_clasz = 9;
  for (auto const& route_node :
       sched_.station_nodes_[station_index]->get_route_nodes()) {
    for (auto const& edge : route_node->edges_) {
      if (edge.empty()) {
        continue;
      }
      auto const& clasz = edge.m_.route_edge_.conns_[0].full_con_->clasz_;
      min_clasz = std::min(clasz, min_clasz);
      if (min_clasz == 0) {
        return 0;
      }
    }
  }
  return min_clasz;
}

std::vector<int64_t> station_matcher::intersect(uint32_t id1, uint32_t id2,
                                                uint8_t clasz) {
  auto const& rels1 = station_rels_[id1];
  auto const& rels2 = station_rels_[id2];
  if (rels1.empty() || rels2.empty()) {
    return {};
  }
  for (auto const& rel1 : rels1) {
    for (auto const& rel2 : rels2) {
      if (rel1.first != rel2.first) {
        continue;
      }
      auto const& route = osm_routes_.at(rel1.first);
      if (route.clasz_ == clasz) {
        return extract_nodes(route.railways_, rel1.second, rel2.second);
      }
    }
  }
  return {};
};

std::vector<int64_t> station_matcher::extract_nodes(
    std::vector<int64_t> const& nodes, int64_t n1, int64_t n2) {
  bool first_found = false;
  std::vector<int64_t> node_section;
  for (auto const& node : nodes) {
    if (node == n1 || node == n2) {
      if (!first_found) {
        first_found = true;
      } else {
        first_found = false;
        node_section.push_back(node);
      }
    }
    if (!first_found) {
      continue;
    }
    node_section.push_back(node);
  }
  return node_section;
};

}  // namespace routes
}  // namespace motis
