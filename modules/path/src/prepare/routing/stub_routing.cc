#include "motis/path/prepare/routing/stub_routing.h"

#include "geo/latlng.h"

namespace motis {
namespace path {

struct stub_routing::impl {
  explicit impl(std::size_t router_id) : router_id_(router_id) {}

  std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from, std::vector<node_ref> const& to) {
    std::vector<std::vector<routing_result>> result;
    for (auto const& f : from) {
      std::vector<routing_result> from_result;
      for (auto const& t : to) {
        source_spec s{0, source_spec::category::UNKNOWN,
                      source_spec::type::STUB_ROUTE};
        from_result.emplace_back(s, router_id_,
                                 distance(f.coords_, t.coords_) + 42);
      }

      result.emplace_back(std::move(from_result));
    }
    return result;
  }

  std::vector<node_ref> close_nodes(geo::latlng const& latlng) {
    std::vector<node_ref> result;
    result.emplace_back(latlng, node_ref_id{}, router_id_);
    return result;
  }

  geo::polyline get_polyline(node_ref const& from, node_ref const& to) {
    return {from.coords_, to.coords_};
  }

  size_t router_id_;
};

stub_routing::stub_routing(std::size_t router_id)
    : impl_(std::make_unique<stub_routing::impl>(router_id)) {}
stub_routing::~stub_routing() = default;

std::vector<std::vector<routing_result>> stub_routing::find_routes(
    std::vector<node_ref> const& from, std::vector<node_ref> const& to) {
  return impl_->find_routes(from, to);
}

std::vector<node_ref> stub_routing::close_nodes(geo::latlng const& latlng) {
  return impl_->close_nodes(latlng);
}

geo::polyline stub_routing::get_polyline(node_ref const& from,
                                         node_ref const& to) {
  return impl_->get_polyline(from, to);
}

}  // namespace path
}  // namespace motis
