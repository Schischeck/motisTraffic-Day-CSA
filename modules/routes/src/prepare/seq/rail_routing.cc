#include "motis/routes/prepare/seq/rail_routing.h"

#include "geo/latlng.h"

#include "motis/routes/prepare/point_rtree.h"
#include "motis/routes/prepare/rail/load_rail_graph.h"
#include "motis/routes/prepare/rail/rail_router.h"

namespace motis {
namespace routes {

struct rail_routing::impl {
  explicit impl(std::size_t router_id, std::string const& path)
      : router_id_(router_id),
        rail_graph_(load_rail_graph(path)),
        rtree_(make_point_rtree(rail_graph_.nodes_, [&](auto&& c) {
          return point_rtree::point{c.pos_.lng_, c.pos_.lat_};
        })) {}

  geo::polyline get_polyline(node_ref const& from, node_ref const& to) {
    std::vector<geo::latlng> polyline;
    return polyline;
  }

  std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from, std::vector<node_ref> const& to) {
    std::vector<std::vector<routing_result>> result;
    std::vector<size_t> to_idx = transform_to_vec(from, [](auto const& ref) {
      if (ref.id_ != -1) {
        return ref.id_;
      }
    });
    for (auto const& f : from) {
      if (from.id_ == -1) {
        continue;
      }
      std::vector<routing_result> from_result;
      rail_graph_dijkstra dijkstra{rail_graph_, {from.id_}, to_idx};
      dijkstra.run();
      for (auto const& t : to_idx) {
        source_spec s(id_, source_spec::category::RAILWAY,
                      source_spec::type::RAIL_ROUTE);
        s.router_id_ = router_id_;
        from_result.emplace_back(s, dijkstra.get_distance(t));
        id_++;
      }
      result.push_back(std::move(from_result));
    }
    return result;
  }

  std::vector<node_ref> close_nodes(node_ref const& station) {
    std::vector<node_ref> refs;
    return refs;
  }

  size_t id_ = 0;
  size_t router_id_;
  rail_graph rail_graph_;
  point_rtree rtree_;
};

rail_routing::rail_routing(std::size_t router_id, std::string path)
    : routing_strategy(router_id),
      impl_(std::make_unique<rail_routing::impl>(router_id, path)) {}
rail_routing::~rail_routing() = default;

std::vector<std::vector<routing_result>> rail_routing::find_routes(
    std::vector<node_ref> const& from, std::vector<node_ref> const& to) {
  return impl_->find_routes(from, to);
}

std::vector<node_ref> rail_routing::close_nodes(node_ref const& station) {
  return impl_->close_nodes(station);
}

geo::polyline rail_routing::get_polyline(node_ref const& from,
                                         node_ref const& to) {
  return impl_->get_polyline(from, to);
}

}  // namespace routes
}  // namespace motis
