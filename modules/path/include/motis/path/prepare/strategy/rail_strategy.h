#pragma once

#include <memory>

#include "motis/path/prepare/strategy/routing_strategy.h"

namespace motis {
namespace path {

// struct rail_strategy : public routing_strategy {

//   explicit rail_strategy(std::size_t router_id, std::string path);
//   ~rail_strategy();

//   std::vector<node_ref> close_nodes(geo::latlng const&) override;

//   std::vector<std::vector<routing_result>> find_routes(
//       std::vector<node_ref> const& from,
//       std::vector<node_ref> const& to) override;

//   geo::polyline get_polyline(node_ref const& from,
//                              node_ref const& to) const override;

// private:
//   struct impl;
//   std::unique_ptr<impl> impl_;
// };

}  // namespace path
}  // motis
