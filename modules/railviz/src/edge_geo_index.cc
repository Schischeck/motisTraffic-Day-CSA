#include "motis/railviz/edge_geo_index.h"

#include <algorithm>
#include <set>
#include <vector>

#include "boost/geometry/geometries/box.hpp"
#include "boost/geometry/geometries/point.hpp"
#include "boost/geometry/geometries/segment.hpp"
#include "boost/geometry/index/rtree.hpp"

#include "motis/core/schedule/schedule.h"

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

typedef bg::cs::spherical_equatorial<bg::degree> coordinate_system;
typedef bg::model::point<double, 2, coordinate_system> spherical_point;
typedef bg::model::box<spherical_point> box;
typedef std::pair<box, int> value;
typedef bgi::rtree<value, bgi::quadratic<16>> rtree;

namespace motis {
namespace railviz {

box bounding_box(spherical_point c1, spherical_point c2) {
  bg::model::segment<spherical_point> s(c1, c2);
  box b;
  bg::envelope(s, b);
  return b;
}

class edge_geo_index::impl {
public:
  impl(int clasz, schedule const& s) : clasz_(clasz), sched_(s) {
    init_rtree();
  }

  std::vector<edge const*> edges(geo::box b) const {
    std::vector<value> result_n;
    auto bounds = bounding_box(spherical_point{b.min().lng, b.min().lat},
                               spherical_point{b.max().lng, b.max().lat});
    rtree_.query(bgi::intersects(bounds), std::back_inserter(result_n));

    std::vector<edge const*> edges;
    for (auto const& station_pair_idx : result_n) {
      retrieve_edges(station_pairs_[station_pair_idx.second], edges);
    }
    return edges;
  }

  geo::box get_bounds() const {
    box b = rtree_.bounds();
    spherical_point bottom_right = b.max_corner();
    spherical_point top_left = b.min_corner();

    geo::coord p1 = {top_left.get<1>(), top_left.get<0>()};
    geo::coord p2 = {bottom_right.get<1>(), bottom_right.get<0>()};
    geo::box box_ = {p1, p2};
    return box_;
  }

private:
  void init_rtree() {
    std::vector<value> entries;
    std::set<std::pair<int, int>> indexed_station_pairs;
    for (const auto& node : sched_.station_nodes) {
      add_edges_of_station(node.get(), indexed_station_pairs, entries);
    }
    rtree_ = rtree(entries);
  }

  void add_edges_of_station(
      station_node const* node,
      std::set<std::pair<int, int>>& indexed_station_pairs,
      std::vector<value>& entries) {
    assert(node->is_station_node());
    for (auto const& route_node : node->get_route_nodes()) {
      add_edges_of_route_node(route_node, indexed_station_pairs, entries);
    }
  }

  void add_edges_of_route_node(
      node const* route_node,
      std::set<std::pair<int, int>>& indexed_station_pairs,
      std::vector<value>& entries) {
    assert(route_node->is_route_node());
    for (auto const& e : route_node->_edges) {
      if (!is_relevant(e)) {
        continue;
      }

      auto station_pair = key(e);
      if (!indexed_station_pairs.insert(station_pair).second) {
        continue;
      }

      entries.push_back(std::make_pair(
          bounding_box(station_coords(e._from), station_coords(e._to)),
          station_pairs_.size()));
      station_pairs_.push_back(station_pair);
    }
  }

  std::pair<int, int> key(edge const& e) const {
    return std::make_pair(e._from->get_station()->_id,
                          e._to->get_station()->_id);
  }

  spherical_point station_coords(node const* n) const {
    auto station_node = n->get_station();
    assert(station_node != nullptr);

    auto station_id = station_node->_id;
    assert(station_id < sched_.stations.size());

    auto const& station = *sched_.stations[station_id].get();
    return spherical_point(station.length, station.width);
  }

  void retrieve_edges(std::pair<int, int> const& station_pair,
                      std::vector<edge const*>& edges) const {
    auto from_station_node = sched_.station_nodes[station_pair.first].get();
    auto to_station_node = sched_.station_nodes[station_pair.second].get();

    for (auto const& route_node : from_station_node->get_route_nodes()) {
      for (auto const& e : route_node->_edges) {
        if (!is_relevant(e) || e._to->get_station() != to_station_node) {
          continue;
        }

        edges.push_back(&e);
      }
    }
  }

  inline bool is_relevant(edge const& e) const {
    return !e.empty() && e._m._route_edge._conns[0]._full_con->clasz == clasz_;
  }

  int clasz_;
  schedule const& sched_;
  std::vector<std::pair<int, int>> station_pairs_;
  rtree rtree_;
};

edge_geo_index::edge_geo_index(int clasz, schedule const& s)
    : impl_(new impl(clasz, s)) {}

edge_geo_index::~edge_geo_index() {}

std::vector<edge const*> edge_geo_index::edges(geo::box area) const {
  return impl_->edges(area);
}

geo::box edge_geo_index::get_bounds() const {
  return impl_.get()->get_bounds();
}

}  // namespace railviz
}  // namespace motis
