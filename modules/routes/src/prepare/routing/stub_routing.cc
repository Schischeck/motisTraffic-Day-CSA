#include "motis/routes/prepare/routing/stub_routing.h"

#include "geo/latlng.h"

namespace motis {
namespace routes {

struct stub_routing::impl {
  explicit impl(std::size_t router_id) : router_id_(router_id) {}

  std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from, std::vector<node_ref> const& to) {
    std::vector<std::vector<routing_result>> result;
    for (auto& f : from) {
      std::vector<routing_result> from_result;

      for (auto& t : to) {
        source_spec s(id_, source_spec::category::UNKNOWN,
                      source_spec::type::STUB_ROUTE);
        s.router_id_ = router_id_;
        from_result.emplace_back(s, distance(f.coords_, t.coords_));
        id_++;
      }

      result.emplace_back(std::move(from_result));
    }
    return result;
  }

  std::vector<node_ref> close_nodes(node_ref const& station) {
    std::vector<node_ref> result;
    result.push_back(station);
    return result;
  }

  geo::polyline get_polyline(node_ref const& from, node_ref const& to) {
    return {from.coords_, to.coords_};
  }

  size_t id_ = 0;
  size_t router_id_;
};

stub_routing::stub_routing(std::size_t router_id)
    : routing_strategy(router_id),
      impl_(std::make_unique<stub_routing::impl>(router_id)) {}
stub_routing::~stub_routing() = default;

std::vector<std::vector<routing_result>> stub_routing::find_routes(
    std::vector<node_ref> const& from, std::vector<node_ref> const& to) {
  return impl_->find_routes(from, to);
}

std::vector<node_ref> stub_routing::close_nodes(node_ref const& station) {
  return impl_->close_nodes(station);
}

geo::polyline stub_routing::get_polyline(node_ref const& from,
                                         node_ref const& to) {
  return impl_->get_polyline(from, to);
}

}  // namespace routes
}  // namespace motis
