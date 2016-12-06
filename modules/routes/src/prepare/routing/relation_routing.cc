#include "motis/routes/prepare/routing/relation_routing.h"

#include "geo/latlng.h"

#include "motis/routes/prepare/point_rtree.h"

namespace motis {
namespace routes {

constexpr auto kMatchRadius = 200;

struct relation_routing::impl {
  explicit impl(std::size_t router_id,
                std::vector<aggregated_polyline> const& polylines)
      : router_id_(router_id),
        polylines_(std::move(polylines)),
        refs_(init_refs()),
        rtree_(init_rtree()) {}

  std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from, std::vector<node_ref> const& to) {
    std::vector<std::vector<routing_result>> result;
    return result;
  }

  std::vector<node_ref> close_nodes(node_ref const& station) {
    std::vector<node_ref> result;
    for (auto const& idx : rtree_.in_radius_with_distance(
             station.coords_.lat_, station.coords_.lng_, kMatchRadius)) {
      result.push_back(refs_[idx.second]);
    }
    return result;
  }

  geo::polyline get_polyline(node_ref const& from, node_ref const& to) {
    return {};
  }

  std::vector<node_ref> init_refs() {
    std::vector<node_ref> refs;
    for (auto const& p : polylines_) {
      for (auto const& latlng : p.polyline_) {
        refs.emplace_back(latlng);
      }
    }
    return refs;
  }

  point_rtree init_rtree() {
    return make_point_rtree(refs_, [&](auto&& c) {
      return point_rtree::point{c.coords_.lng_, c.coords_.lat_};
    });
  }

  size_t id_ = 0;
  size_t router_id_;
  std::vector<aggregated_polyline> polylines_;
  std::vector<node_ref> refs_;
  point_rtree rtree_;
};

relation_routing::relation_routing(
    std::size_t router_id, std::vector<aggregated_polyline> const& polylines)
    : routing_strategy(router_id),
      impl_(std::make_unique<relation_routing::impl>(router_id, polylines)) {}
relation_routing::~relation_routing() = default;

std::vector<std::vector<routing_result>> relation_routing::find_routes(
    std::vector<node_ref> const& from, std::vector<node_ref> const& to) {
  return impl_->find_routes(from, to);
}

std::vector<node_ref> relation_routing::close_nodes(node_ref const& station) {
  return impl_->close_nodes(station);
}

geo::polyline relation_routing::get_polyline(node_ref const& from,
                                             node_ref const& to) {
  return impl_->get_polyline(from, to);
}

}  // namespace routes
}  // namespace motis
