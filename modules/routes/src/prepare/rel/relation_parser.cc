#include "motis/routes/prepare/rel/relation_parser.h"

#include "motis/core/common/get_or_create.h"
#include "motis/core/common/logging.h"
#include "motis/core/common/transform_to_vec.h"

#include "motis/routes/prepare/osm_util.h"
#include "motis/routes/prepare/vector_utils.h"

using namespace motis::logging;

namespace motis {
namespace routes {

parsed_relations parse_relations(std::string const& osm_file_) {
  scoped_timer scoped_timer("parse osm relations");

  parsed_relations result;

  std::map<int64_t, way*> pending_ways_;
  std::map<int64_t, std::vector<node*>> pending_nodes_;

  std::vector<std::string> types{"route", "public_transport"};
  std::vector<std::string> rail_routes{"railway", "train", "light_rail"};
  std::vector<std::string> other_routes{"subway", "bus", "tram"};

  foreach_osm_relation(osm_file_, [&](auto&& relation) {
    auto const type = relation.get_value_by_key("type", "");
    if (std::none_of(begin(types), end(types),
                     [&](auto&& t) { return t == type; })) {
      return;
    }

    auto const route = relation.get_value_by_key("route", "");
    auto const is_rail = std::any_of(begin(rail_routes), end(rail_routes),
                                     [&](auto&& r) { return r == route; });

    if (!is_rail && std::none_of(begin(other_routes), end(other_routes),
                                 [&](auto&& r) { return r == route; })) {
      return;
    }

    std::vector<way*> ways;
    for (auto const& member : relation.members()) {
      if (member.type() != osmium::item_type::way) {
        continue;
      }

      ways.push_back(get_or_create(pending_ways_, member.ref(), [&]() {
        result.way_mem_.push_back(std::make_unique<way>(member.ref()));
        return result.way_mem_.back().get();
      }));
    }

    auto cat = is_rail ? source_spec::category::RAILWAY
                       : source_spec::category::UNKNOWN;
    result.relations_.push_back({{relation.id(), cat, source_spec::type::RELATION}, ways});
  });

  std::string platform = "platform";
  std::string stop = "stop";

  foreach_osm_way(osm_file_, [&](auto&& way) {
    auto w = pending_ways_.find(way.id());
    if (w == end(pending_ways_) ||
        platform == way.get_value_by_key("highway", "") ||
        platform == way.get_value_by_key("public_transport", "") ||
        stop == way.get_value_by_key("role", "")) {
      return;
    }

    w->second->resolved_ = true;
    w->second->nodes_ =
        transform_to_vec(std::begin(way.nodes()), std::end(way.nodes()),
                         [](auto&& n) { return node{n.ref()}; });

    for (auto& n : w->second->nodes_) {
      pending_nodes_[n.id_].push_back(&n);
    }
  });

  foreach_osm_node(osm_file_, [&](auto&& node) {
    auto it = pending_nodes_.find(node.id());
    if (it != end(pending_nodes_)) {
      for (auto& n : it->second) {
        n->resolved_ = true;
        n->pos_ = {node.location().lat(), node.location().lon()};
      }
    }
  });

  // post processing
  for (auto& rel : result.relations_) {
    auto& ways = rel.ways_;
    for (auto& way : ways) {
      for (auto& node : way->nodes_) {
        if (!node.resolved_) {
          std::cout << "missing node" << node.id_ << std::endl;
        }
      }
    }

    erase_if(ways, [](auto const& w) {
      return !w->resolved_ || w->nodes_.size() < 2;
    });
  }

  return result;
}

}  // namespace routes
}  // namespace motis
