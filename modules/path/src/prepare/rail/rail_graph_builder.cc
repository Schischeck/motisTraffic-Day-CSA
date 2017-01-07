#include "motis/path/prepare/rail/rail_graph_builder.h"

#include <algorithm>
#include <map>

#include "parser/util.h"

#include "utl/concat.h"
#include "utl/erase_duplicates.h"
#include "utl/to_vec.h"

#include "motis/core/common/hash_map.h"
#include "motis/core/common/logging.h"

#include "motis/path/prepare/osm_util.h"

using namespace motis::logging;
using namespace geo;

namespace motis {
namespace path {

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

struct temp_edge {
  temp_edge(size_t id, int64_t from, int64_t to, geo::polyline polyline)
      : valid_(true),
        ids_({id}),
        from_(from),
        to_(to),
        polyline_(std::move(polyline)) {}

  bool valid_;
  std::vector<size_t> ids_;
  int64_t from_, to_;
  geo::polyline polyline_;
};

struct rail_graph_builder {
  rail_graph_builder() {
    osm_nodes_.set_empty_key(std::numeric_limits<int64_t>::min());
  }

  void extract_rail_ways(std::string const& osm_file) {
    scoped_timer timer("parsing osm rail ways");

    std::string rail{"rail"};
    std::string yes{"yes"};
    std::string crossover{"crossover"};
    std::vector<std::string> excluded_usages{"industrial", "military", "test",
                                             "tourism"};
    std::vector<std::string> excluded_services{"yard", "spur"};  // , "siding"

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

      if (yes == way.get_value_by_key("railway:preserved", "")) {
        return;
      }

      osm_ways_.emplace_back(
          way.id(),
          utl::to_vec(way.nodes(), [&](auto const& node_ref) -> osm_rail_node* {
            auto it = osm_nodes_.find(node_ref.ref());
            if (it == end(osm_nodes_)) {
              osm_node_mem_.emplace_back(
                  std::make_unique<osm_rail_node>(node_ref.ref(), way.id()));
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

  void make_temp_edges() {
    for (auto& node : osm_node_mem_) {
      utl::erase_duplicates(node->in_ways_);
    }

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

        extract_temp_edge(way.id_, from, to);
        from = to;
      }

      if (std::distance(from, end(way.nodes_)) > 2) {
        extract_temp_edge(way.id_, from, std::next(end(way.nodes_), -1));
      }
    }
  }

  void extract_temp_edge(int64_t const id,
                         std::vector<osm_rail_node*>::const_iterator const from,
                         std::vector<osm_rail_node*>::const_iterator const to) {
    std::vector<latlng> polyline;
    polyline.reserve(std::distance(from, to));
    for (auto it = from; it != std::next(to); ++it) {
      polyline.emplace_back((*it)->pos_.lat_, (*it)->pos_.lng_);
    }

    if (polyline.size() < 2) {
      return;
    }

    temp_edges_.emplace_back(id, (*from)->id_, (*to)->id_, std::move(polyline));
  }

  void report_cycle_detected(temp_edge const& edge, int64_t const where) {
    LOG(motis::logging::info) << "rail graph: maybe cycle detected @ node "
                              << where << " -- (" << edge.from_ << "->"
                              << edge.to_ << ") ";
    for (auto const& id : edge.ids_) {
      std::cout << id << ", ";
    }
  }

  void aggregate_temp_edges() {
    hash_map<int64_t, size_t> degrees;
    degrees.set_empty_key(std::numeric_limits<int64_t>::max());
    for (auto const& edge : temp_edges_) {
      degrees[edge.from_] += 1;
      degrees[edge.to_] += 1;
    }

    for (auto it = begin(temp_edges_); it != end(temp_edges_); ++it) {
      if (!it->valid_ || it->from_ == it->to_) {
        continue;
      }

      while (degrees[it->from_] == 2) {
        if (it->from_ == it->to_) {
          report_cycle_detected(*it, it->from_);
          break;
        }

        auto other_it = std::find_if(
            std::next(it), end(temp_edges_), [&](auto const& other) {
              return other.valid_ &&
                     (it->from_ == other.from_ || it->from_ == other.to_);
            });

        verify(other_it != end(temp_edges_), "rail: node not found (from)");

        if (it->from_ == other_it->to_) {
          //  --(other)--> X --(this)-->
          it->from_ = other_it->from_;
        } else {
          //  <--(other)-- X --(this)-->
          it->from_ = other_it->to_;

          std::reverse(begin(other_it->polyline_), end(other_it->polyline_));
        }

        utl::concat(other_it->polyline_, it->polyline_);
        it->polyline_ = std::move(other_it->polyline_);

        utl::concat(it->ids_, other_it->ids_);
        other_it->valid_ = false;
      }

      if (it->from_ == it->to_) {
        break;
      }

      while (degrees[it->to_] == 2) {
        if (it->from_ == it->to_) {
          report_cycle_detected(*it, it->to_);
          break;
        }

        auto other_it = std::find_if(
            std::next(it), end(temp_edges_), [&](auto const& other) {
              return other.valid_ &&
                     (it->to_ == other.from_ || it->to_ == other.to_);
            });

        verify(other_it != end(temp_edges_), "rail: node not found (to)");

        if (it->to_ == other_it->from_) {
          // --(this)--> X --(other)-->
          it->to_ = other_it->to_;
        } else {
          // --(this)--> X <--(other)--
          it->to_ = other_it->from_;
          std::reverse(begin(other_it->polyline_), end(other_it->polyline_));
        }

        utl::concat(it->polyline_, other_it->polyline_);
        utl::concat(it->ids_, other_it->ids_);
        other_it->valid_ = false;
      }
    }
  }

  void build_graph() {
    hash_map<int64_t, rail_node*> node_map;
    node_map.set_empty_key(std::numeric_limits<int64_t>::min());

    for (auto const& e : temp_edges_) {
      if (!e.valid_) {
        continue;
      }

      if(e.from_ == e.to_) {
        report_cycle_detected(e, 42);
      }

      auto& nodes = graph_.nodes_;
      auto from = map_get_or_create(node_map, e.from_, [&] {
        nodes.emplace_back(std::make_unique<rail_node>(nodes.size(), e.from_));
        return nodes.back().get();
      });

      auto to = map_get_or_create(node_map, e.to_, [&] {
        nodes.emplace_back(std::make_unique<rail_node>(nodes.size(), e.to_));
        return nodes.back().get();
      });

      auto const info_idx = graph_.infos_.size();
      auto const dist = length(e.polyline_);

      from->edges_.emplace_back(info_idx, true, dist, from, to);
      to->edges_.emplace_back(info_idx, false, dist, to, from);

      graph_.infos_.emplace_back(std::make_unique<rail_edge_info>(
          std::move(e.polyline_), dist, from, to));
    }
  }

  std::vector<std::unique_ptr<osm_rail_node>> osm_node_mem_;
  hash_map<int64_t, osm_rail_node*> osm_nodes_;
  std::vector<osm_rail_way> osm_ways_;

  std::vector<temp_edge> temp_edges_;

  rail_graph graph_;
};

rail_graph build_rail_graph(std::string const& osm_file) {
  rail_graph_builder builder;
  builder.extract_rail_ways(osm_file);
  builder.extract_rail_nodes(osm_file);

  scoped_timer timer("building rail graph");

  builder.make_temp_edges();
  builder.aggregate_temp_edges();

  builder.build_graph();

  // dump_rail_graph(builder.graph_);

  return std::move(builder.graph_);
}

}  // namespace path
}  // namespace motis
