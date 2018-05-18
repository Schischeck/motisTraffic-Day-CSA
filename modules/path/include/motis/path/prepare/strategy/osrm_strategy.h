#pragma once

#include <memory>

#include "motis/path/prepare/schedule/stations.h"
#include "motis/path/prepare/strategy/routing_strategy.h"

namespace motis {
namespace path {

struct osrm_strategy : public routing_strategy {
  osrm_strategy(strategy_id_t, std::vector<station> const&,
                std::string const& osrm_path);
  osrm_strategy(osrm_strategy const&) = delete;
  osrm_strategy(osrm_strategy&&) = default;
  osrm_strategy& operator=(osrm_strategy const&) = delete;
  osrm_strategy& operator=(osrm_strategy&&) = default;
  ~osrm_strategy() override;

  std::vector<node_ref> close_nodes(
      std::string const& station_id) const override;

  bool is_cacheable() const override { return true; }

  bool can_route(node_ref const&) const override;

  routing_result_matrix find_routes(
      std::vector<node_ref> const& from,
      std::vector<node_ref> const& to) const override;

  geo::polyline get_polyline(node_ref const& from,
                             node_ref const& to) const override;

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace path
}  // namespace motis
