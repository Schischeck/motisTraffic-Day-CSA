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
    std::vector<std::vector<routing_result>> result{from.size()};
    for (auto const& f : from) {
      if (f.router_id_ != router_id_) {
        result.push_back({});
        continue;
      }
      std::vector<routing_result> from_result;
      for (auto const& t : to) {
        if (t.router_id_ != router_id_ ||
            f.id_.relation_id_ != t.id_.relation_id_) {
          continue;
        }
        auto p = polylines_[f.id_.relation_id_];
        from_result.emplace_back(p.source_, router_id_, 0);
      }
      result.push_back(std::move(from_result));
    }
    return result;
  }

  std::vector<node_ref> close_nodes(geo::latlng const& latlng) {
    std::vector<node_ref> result;
    auto nodes =
        rtree_.in_radius_with_distance(latlng.lat_, latlng.lng_, kMatchRadius);
    std::vector<std::size_t> visited;
    for (auto i = 0u; i < (nodes.size() < 5 ? nodes.size() : 5); ++i) {
      auto& ref = refs_[nodes[i].second];
      if (std::find(begin(visited), end(visited), ref.id_.relation_id_) !=
          end(visited)) {
        continue;
      }
      visited.push_back(ref.id_.relation_id_);
      result.push_back(refs_[nodes[i].second]);
    }
    return result;
  }

  geo::polyline get_polyline(node_ref const& from, node_ref const& to) {
    if (from.router_id_ != router_id_ || to.router_id_ != router_id_ ||
        from.id_.relation_id_ != to.id_.relation_id_) {
      return {};
    }
    auto p = polylines_[from.id_.relation_id_];
    geo::polyline polyline;
    polyline.insert(
        begin(polyline),
        begin(p.polyline_) + std::min(from.id_.id_, to.id_.id_),
        begin(p.polyline_) + std::max(from.id_.id_, to.id_.id_) + 1);
    return polyline;
  }

  std::vector<node_ref> init_refs() {
    std::vector<node_ref> refs;
    for (auto i = 0u; i < polylines_.size(); ++i) {
      for (auto j = 0u; j < polylines_[i].polyline_.size(); ++j) {
        refs.emplace_back(polylines_[i].polyline_[j], node_ref_id(i, j),
                          router_id_);
      }
    }
    return refs;
  }

  point_rtree init_rtree() {
    return make_point_rtree(refs_, [&](auto&& c) {
      return point_rtree::point{c.coords_.lng_, c.coords_.lat_};
    });
  }

  size_t router_id_;
  std::vector<aggregated_polyline> polylines_;
  std::vector<node_ref> refs_;
  point_rtree rtree_;
};

relation_routing::relation_routing(
    std::size_t router_id, std::vector<aggregated_polyline> const& polylines)
    : impl_(std::make_unique<relation_routing::impl>(router_id, polylines)) {}
relation_routing::~relation_routing() = default;

std::vector<std::vector<routing_result>> relation_routing::find_routes(
    std::vector<node_ref> const& from, std::vector<node_ref> const& to) {
  return impl_->find_routes(from, to);
}

std::vector<node_ref> relation_routing::close_nodes(geo::latlng const& latlng) {
  return impl_->close_nodes(latlng);
}

geo::polyline relation_routing::get_polyline(node_ref const& from,
                                             node_ref const& to) {
  return impl_->get_polyline(from, to);
}

}  // namespace routes
}  // namespace motis
