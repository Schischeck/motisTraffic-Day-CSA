#include "motis/railviz/edge_geo_index.h"

#include <vector>
#include <algorithm>

#include "boost/geometry/geometries/point.hpp"
#include "boost/geometry/geometries/segment.hpp"
#include "boost/geometry/geometries/box.hpp"
#include "boost/geometry/index/rtree.hpp"

#include "motis/core/schedule/schedule.h"

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

typedef bg::cs::spherical_equatorial<bg::degree> coordinate_system;
typedef bg::model::point<double, 2, coordinate_system> spherical_point;
typedef bg::model::box<spherical_point> box;
typedef std::pair<box, int> value;

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
    auto bounds = bounding_box(spherical_point{b.min.lng, b.min.lat},
                               spherical_point{b.max.lng, b.max.lat});
    rtree_.query(bgi::intersects(bounds), std::back_inserter(result_n));

    std::vector<edge const*> edges(result_n.size());
    std::transform(
        begin(result_n), end(result_n), begin(edges),
        [this](value const& result) { return edges_[result.second]; });
    return edges;
  }

private:
  void init_rtree() {
    for (const auto& node : sched_.station_nodes) {
      add_edges_of_station(node.get());
    }
  }

  void add_edges_of_station(station_node const* node) {
    assert(node->is_station_node());
    for (auto const& route_node : node->get_route_nodes()) {
      add_edges_of_route_node(route_node);
    }
  }

  void add_edges_of_route_node(node const* route_node) {
    assert(route_node->is_route_node());
    for (auto const& e : route_node->_edges) {
      if (e.empty() || e._m._route_edge._conns[0]._full_con->clasz != clasz_) {
        continue;
      }
      rtree_.insert(std::make_pair(
          bounding_box(station_coords(e._from), station_coords(e._to)),
          edges_.size()));
      edges_.push_back(&e);
    }
  }

  spherical_point station_coords(node const* n) const {
    auto station_node = n->get_station();
    assert(station_node != nullptr);

    auto station_id = station_node->_id;
    assert(station_id < sched_.stations.size());

    auto const& station = *sched_.stations[station_id].get();
    return spherical_point(station.length, station.width);
  }

  int clasz_;
  schedule const& sched_;
  std::vector<edge const*> edges_;
  bgi::rtree<value, bgi::quadratic<16>> rtree_;
};

edge_geo_index::edge_geo_index(int clasz, schedule const& s)
    : impl_(new impl(clasz, s)) {}

edge_geo_index::~edge_geo_index() {}

std::vector<edge const*> edge_geo_index::edges(geo::box area) const {
  return impl_->edges(area);
}

}  // namespace railviz
}  // namespace motis