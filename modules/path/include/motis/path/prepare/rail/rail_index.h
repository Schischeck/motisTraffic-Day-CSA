#pragma once

namespace motis {
namespace path {

struct rail_phantom_node {
  size_t info_idx_;
  size_t point_idx_;

  size_t dist_;  // from start
  geo::latlng pos_;
};

struct rail_index {

  rail_index(rail_graph const& graph) : graph_(graph) {

    std::vector<geo::latlng>


  }

      std::vector<rail_phantom_node> get_rail_phantoms(
          geo::latlng const& pos, double const radius) const {}


  rail_graph const& graph_;

  std::vector<std::pair<size_t, size_t>> nodes_;


};

}  // namespace path
}  // namespace motis