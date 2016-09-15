#include "motis/routes/prepare/rail/load_rail_graph.h"

#include <algorithm>
#include <map>

#include "motis/core/common/geo.h"
#include "motis/core/common/hash_map.h"
#include "motis/core/common/logging.h"
#include "motis/core/common/transform_to_vec.h"

#include "motis/routes/prepare/osm_util.h"
#include "motis/routes/prepare/rail/dump_rail_graph.h"

using namespace motis::logging;

namespace motis {
namespace routes {

struct osm_rail_node {
  explicit osm_rail_node(int64_t id, int64_t way) : id_(id) {
    in_ways_.push_back(way);
  }

  int64_t id_;
  latlng pos_;
  std::vector<int64_t> in_ways_;
};

struct osm_rail_way {
  osm_rail_way(int64_t id, std::vector<osm_rail_node*> nodes)
      : id_(id), nodes_(std::move(nodes)) {}

  int64_t id_;
  std::vector<osm_rail_node*> nodes_;
};

size_t polyline_length(std::vector<latlng> const& polyline) {
  size_t length = 0;
  for (auto i = 0u; i < polyline.size() - 1; ++i) {
    length += geo_detail::distance_in_m(polyline[i], polyline[i + 1]);
  }
  return length;
}

struct rail_graph_loader {
  rail_graph_loader() {
    osm_nodes_.set_empty_key(std::numeric_limits<int64_t>::min());
    graph_nodes_.set_empty_key(std::numeric_limits<int64_t>::min());
  }

  void extract_rail_ways(std::string const& osm_file) {
    scoped_timer timer("parsing osm rail ways");

    std::string rail{"rail"};
    std::string yes{"yes"};
    std::string crossover{"crossover"};
    std::vector<std::string> excluded_usages{"industrial", "military", "test",
                                             "tourism"};
    std::vector<std::string> excluded_services{"yard", "spur"}; // , "siding"

    foreach_osm_way(osm_file, [&](auto&& way) {
      if (rail != way.get_value_by_key("railway", "")) {
        return;
      }

      auto const usage = way.get_value_by_key("usage", "");
      if (std::any_of(begin(excluded_usages), end(excluded_usages),
                      [&](auto&& u) { return u == usage; })) {
        return;
      }

      auto const service = way.get_value_by_key("service", "");
      if (std::any_of(begin(excluded_services), end(excluded_services),
                      [&](auto&& s) { return s == service; })) {
        return;
      }

      if(yes == way.get_value_by_key("railway:preserved", "")) {
        return;
      }

      osm_ways_.emplace_back(
          way.id(),
          transform_to_vec(
              way.nodes(), [&](auto const& node_ref) -> osm_rail_node* {
                auto it = osm_nodes_.find(node_ref.ref());
                if (it == end(osm_nodes_)) {
                  osm_node_mem_.emplace_back(std::make_unique<osm_rail_node>(
                      node_ref.ref(), way.id()));
                  osm_nodes_[node_ref.ref()] = osm_node_mem_.back().get();
                  return osm_node_mem_.back().get();
                } else {
                  it->second->in_ways_.push_back(way.id());
                  return it->second;
                }
              }));
    });
  }

  void extract_rail_nodes(std::string const& osm_file) {
    scoped_timer timer("parsing osm rail nodes");
    foreach_osm_node(osm_file, [&](auto&& node) {
      auto it = osm_nodes_.find(node.id());
      if (it == end(osm_nodes_)) {
        return;
      }

      it->second->pos_.lat_ = node.location().lat();
      it->second->pos_.lng_ = node.location().lon();
    });
  }

  void build_graph() {
    scoped_timer timer("building rail graph");

    for (auto const& way : osm_ways_) {
      if (way.nodes_.size() < 2) {
        continue;
      }

      auto from = begin(way.nodes_);
      while (true) {
        auto to = std::find_if(
            std::next(from), end(way.nodes_),
            [](auto const& node) { return node->in_ways_.size() > 1; });

        if (to == end(way.nodes_)) {
          break;
        }

        connect_nodes(way.id_, from, to);
        from = to;
      }

      if (std::distance(from, end(way.nodes_)) > 2) {
        connect_nodes(way.id_, from, std::next(end(way.nodes_), -1));
      }
    }
  }

  void connect_nodes(int64_t const id,
                     std::vector<osm_rail_node*>::const_iterator const from,
                     std::vector<osm_rail_node*>::const_iterator const to) {
    std::vector<latlng> polyline;
    polyline.reserve(std::distance(from, to));
    for (auto it = from; it != std::next(to); ++it) {
      polyline.emplace_back((*it)->pos_.lat_, (*it)->pos_.lng_);
    }
    auto const dist = polyline_length(polyline);

    auto& nodes = graph_.nodes_;
    auto from_node = map_get_or_create(graph_nodes_, (*from)->id_, [&]() {
      nodes.emplace_back(std::make_unique<rail_node>(nodes.size(), (*from)->id_,
                                                     (*from)->pos_));
      return nodes.back().get();
    });

    auto to_node = map_get_or_create(graph_nodes_, (*to)->id_, [&]() {
      nodes.emplace_back(
          std::make_unique<rail_node>(nodes.size(), (*to)->id_, (*to)->pos_));
      return nodes.back().get();
    });

    from_node->links_.emplace_back(id, polyline, dist, from_node, to_node);

    std::reverse(begin(polyline), end(polyline));
    to_node->links_.emplace_back(id, polyline, dist, to_node, from_node);
  }

  std::vector<std::unique_ptr<osm_rail_node>> osm_node_mem_;
  hash_map<int64_t, osm_rail_node*> osm_nodes_;
  std::vector<osm_rail_way> osm_ways_;

  rail_graph graph_;
  hash_map<int64_t, rail_node*> graph_nodes_;
};


rail_graph load_rail_graph(std::string const& osm_file) {
  rail_graph_loader loader;
  loader.extract_rail_ways(osm_file);
  loader.extract_rail_nodes(osm_file);
  loader.build_graph();

  dump_rail_graph(loader.graph_);

  return std::move(loader.graph_);
}

}  // namespace routes
}  // namespace motis
