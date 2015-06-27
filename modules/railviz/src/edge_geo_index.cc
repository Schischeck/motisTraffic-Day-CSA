#include "motis/railviz/edge_geo_index.h"

#include <vector>

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
  impl(schedule const& s) : sched_(s) { init_rtree(); }

  std::vector<edge const*> edges(double top_left_lat, double top_left_lng,
                                 double bottom_right_lat,
                                 double bottom_right_lng) const {
    spherical_point top_left(top_left_lng, top_left_lat);
    spherical_point bottom_right(bottom_right_lng, bottom_right_lat);

    std::vector<value> result_n;
    rtree_.query(bgi::intersects(box(bottom_right, top_left)),
                 std::back_inserter(result_n));

    std::vector<edge const*> edges;
    for (const auto& result : result_n) {
      edges.push_back(edges_[result.second]);
    }
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
      if (e.empty()) {
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

  schedule const& sched_;
  std::vector<edge const*> edges_;
  bgi::rtree<value, bgi::quadratic<16>> rtree_;
};

edge_geo_index::edge_geo_index(schedule const& s) : impl_(new impl(s)) {}

edge_geo_index::~edge_geo_index() {}

std::vector<edge const*> edge_geo_index::edges(double top_left_lat,
                                               double top_left_lng,
                                               double bottom_right_lat,
                                               double bottom_right_lng) const {
  return impl_->edges(top_left_lat, top_left_lng, bottom_right_lat,
                      bottom_right_lng);
}

}  // namespace railviz
}  // namespace motis