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

spherical_point to_point(geo::coord const c) {
  return spherical_point{c.lng_, c.lat_};
}

geo::coord to_coord(spherical_point const p) {
  return {p.get<0>(), p.get<1>()};
}

box bounding_box(spherical_point const c1, spherical_point const c2) {
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

  std::vector<edge const*> edges(geo::box const b) const {
    std::vector<value> result_n;
    auto const bounds = bounding_box(to_point(b.first), to_point(b.second));
    rtree_.query(bgi::intersects(bounds), std::back_inserter(result_n));

    std::vector<edge const*> edges;
    for (auto const& station_pair_idx : result_n) {
      retrieve_edges(station_pairs_[station_pair_idx.second], edges);
    }
    return edges;
  }

  geo::box get_bounds() const {
    auto const bounds = rtree_.bounds();
    return {to_coord(bounds.min_corner()), to_coord(bounds.max_corner())};
  }

private:
  static std::pair<int, int> key(edge const& e) {
    return {e.from_->get_station()->id_, e.to_->get_station()->id_};
  }

  void init_rtree() {
    std::vector<value> entries;
    std::set<std::pair<int, int>> indexed_station_pairs;
    for (const auto& node : sched_.station_nodes_) {
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
    for (auto const& e : route_node->edges_) {
      if (!is_relevant(e)) {
        continue;
      }

      auto const station_pair = key(e);
      if (!indexed_station_pairs.insert(station_pair).second) {
        continue;
      }

      entries.push_back(std::make_pair(
          bounding_box(station_coords(e.from_), station_coords(e.to_)),
          station_pairs_.size()));
      station_pairs_.push_back(station_pair);
    }
  }

  void retrieve_edges(std::pair<int, int> const& station_pair,
                      std::vector<edge const*>& edges) const {
    auto const from = sched_.station_nodes_[station_pair.first].get();
    auto const to = sched_.station_nodes_[station_pair.second].get();

    for (auto const& route_node : from->get_route_nodes()) {
      for (auto const& e : route_node->edges_) {
        if (!is_relevant(e) || e.to_->get_station() != to) {
          continue;
        }

        edges.push_back(&e);
      }
    }
  }

  spherical_point station_coords(node const* n) {
    auto station_node = n->get_station();
    assert(station_node != nullptr);

    auto station_id = station_node->id_;
    assert(station_id < sched_.stations_.size());

    auto const& station = *sched_.stations_[station_id].get();
    return spherical_point(station.length_, station.width_);
  }

  inline bool is_relevant(edge const& e) const {
    return !e.empty() && e.m_.route_edge_.conns_[0].full_con_->clasz_ == clasz_;
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
