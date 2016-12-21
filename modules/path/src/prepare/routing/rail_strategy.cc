#include "motis/path/prepare/routing/rail_strategy.h"

#include "geo/latlng.h"
#include "geo/point_rtree.h"

#include "motis/path/prepare/rail/load_rail_graph.h"
#include "motis/path/prepare/rail/rail_router.h"

using namespace geo;

namespace motis {
namespace path {

struct rail_strategy::impl {
  explicit impl(std::size_t router_id, std::string const& path)
      : router_id_(router_id),
        rail_graph_(load_rail_graph(path)),
        rtree_(make_point_rtree(rail_graph_.nodes_,
                                [&](auto const& c) { return c->pos_; })) {}

  geo::polyline get_polyline(node_ref const& from, node_ref const& to) {
    std::vector<geo::latlng> polyline;
    return {polyline};
  }

  std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from, std::vector<node_ref> const& to) {
    std::vector<std::vector<routing_result>> result;
    return result;
  }

  std::vector<node_ref> close_nodes(geo::latlng const& latlng) {
    std::vector<node_ref> refs;
    return refs;
  }

  size_t id_ = 0;
  size_t router_id_;
  rail_graph rail_graph_;
  point_rtree rtree_;
};

rail_strategy::rail_strategy(std::size_t router_id, std::string path)
    : impl_(std::make_unique<rail_strategy::impl>(router_id, path)) {}
rail_strategy::~rail_strategy() = default;

std::vector<std::vector<routing_result>> rail_strategy::find_routes(
    std::vector<node_ref> const& from, std::vector<node_ref> const& to) {
  return impl_->find_routes(from, to);
}

std::vector<node_ref> rail_strategy::close_nodes(geo::latlng const& latlng) {
  return impl_->close_nodes(latlng);
}

geo::polyline rail_strategy::get_polyline(node_ref const& from,
                                         node_ref const& to) {
  return impl_->get_polyline(from, to);
}

}  // namespace path
}  // namespace motis
