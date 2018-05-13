#include "motis/path/prepare/rail/rail_way.h"

#include <algorithm>
#include <limits>
#include <map>

#include "parser/util.h"

#include "utl/concat.h"
#include "utl/erase_duplicates.h"
#include "utl/erase_if.h"
#include "utl/to_vec.h"

#include "motis/core/common/hash_map.h"
#include "motis/core/common/logging.h"

#include "motis/path/prepare/osm_util.h"

using namespace motis::logging;
using namespace geo;

namespace motis {
namespace path {

struct osm_rail_node {
  osm_rail_node(int64_t id, int64_t way) : id_(id), in_ways_({way}) {}

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

struct builder {
  builder() { osm_nodes_.set_empty_key(std::numeric_limits<int64_t>::min()); }

  template <typename Fun>
  void extract_osm_ways(std::string const& osm_file, Fun&& is_valid) {
    scoped_timer timer("parsing osm rail ways");

    foreach_osm_way(osm_file, [&](auto&& way) {
      if (!is_valid(way)) {
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

  void extract_osm_nodes(std::string const& osm_file) {
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

  void make_rail_ways() {
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

        extract_rail_way(way.id_, from, to);
        from = to;
      }

      if (std::distance(from, end(way.nodes_)) > 2) {
        extract_rail_way(way.id_, from, std::next(end(way.nodes_), -1));
      }
    }
  }

  void extract_rail_way(int64_t const id,
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

    rail_ways_.emplace_back((*from)->id_, (*to)->id_, id, std::move(polyline));
  }

  void report_cycle_detected(rail_way const& edge, int64_t const where) {
    LOG(motis::logging::info)
        << "rail graph: maybe cycle detected @ node " << where << " -- ("
        << edge.from_ << "->" << edge.to_ << ") ";
    for (auto const& id : edge.ids_) {
      std::cout << id << ", ";
    }
  }

  void aggregate_rail_ways() {
    hash_map<int64_t, size_t> degrees;
    degrees.set_empty_key(std::numeric_limits<int64_t>::max());
    for (auto const& way : rail_ways_) {
      verify(way.is_valid(), "initially all ways must be valid!");
      degrees[way.from_] += 1;
      degrees[way.to_] += 1;
    }

    for (auto it = begin(rail_ways_); it != end(rail_ways_); ++it) {
      if (!it->is_valid() || it->from_ == it->to_) {
        continue;
      }

      while (degrees[it->from_] == 2) {
        if (it->from_ == it->to_) {
          report_cycle_detected(*it, it->from_);
          break;
        }

        auto other_it = std::find_if(
            std::next(it), end(rail_ways_), [&](auto const& other) {
              return other.is_valid() &&
                     (it->from_ == other.from_ || it->from_ == other.to_);
            });

        verify(other_it != end(rail_ways_), "rail: node not found (from)");

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
        other_it->invalidate();
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
            std::next(it), end(rail_ways_), [&](auto const& other) {
              return other.is_valid() &&
                     (it->to_ == other.from_ || it->to_ == other.to_);
            });

        verify(other_it != end(rail_ways_), "rail: node not found (to)");

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
        other_it->invalidate();
      }
    }

    utl::erase_if(rail_ways_,
                  [](auto const& rail_way) { return !rail_way.is_valid(); });
  }

  std::vector<std::unique_ptr<osm_rail_node>> osm_node_mem_;
  hash_map<int64_t, osm_rail_node*> osm_nodes_;
  std::vector<osm_rail_way> osm_ways_;

  std::vector<rail_way> rail_ways_;
};

template <typename Fun>
std::vector<rail_way> build_rail_ways(std::string const& osm_file,
                                      Fun&& is_valid) {
  builder b;

  b.extract_osm_ways(osm_file, is_valid);
  b.extract_osm_nodes(osm_file);

  b.make_rail_ways();
  b.aggregate_rail_ways();

  return std::move(b.rail_ways_);
}

std::vector<rail_way> build_rail_ways(std::string const& osm_file) {
  std::string rail{"rail"};
  std::string yes{"yes"};
  std::vector<std::string> excluded_usages{"industrial", "military", "test",
                                           "tourism"};
  std::vector<std::string> excluded_services{"yard", "spur"};  // , "siding"
  auto const& is_valid = [&](auto&& way) {
    if (rail != way.get_value_by_key("railway", "")) {
      return false;
    }

    auto const usage = way.get_value_by_key("usage", "");
    if (std::any_of(begin(excluded_usages), end(excluded_usages),
                    [&](auto&& u) { return u == usage; })) {
      return false;
    }

    auto const service = way.get_value_by_key("service", "");
    if (std::any_of(begin(excluded_services), end(excluded_services),
                    [&](auto&& s) { return s == service; })) {
      return false;
    }

    if (yes == way.get_value_by_key("railway:preserved", "")) {
      return false;
    }
    return true;
  };
  return build_rail_ways(osm_file, is_valid);
}

std::vector<rail_way> build_sub_ways(std::string const& osm_file) {
  std::vector<std::string> rail_route{"light_rail", "subway"};
  std::vector<std::string> excluded_usages{"industrial", "military", "test",
                                           "tourism"};
  auto const& is_valid = [&](auto&& way) {
    auto const rail = way.get_value_by_key("railway", "");
    if (std::none_of(begin(rail_route), end(rail_route),
                     [&](auto&& r) { return r == rail; })) {
      return false;
    }

    auto const usage = way.get_value_by_key("usage", "");
    if (std::any_of(begin(excluded_usages), end(excluded_usages),
                    [&](auto&& u) { return u == usage; })) {
      return false;
    }
    return true;
  };
  return build_rail_ways(osm_file, is_valid);
}

}  // namespace path
}  // namespace motis
