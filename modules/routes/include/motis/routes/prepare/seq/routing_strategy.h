#pragma once

#include "motis/geo/latlng.h"
#include "motis/geo/polyline.h"

namespace motis {
namespace routes {

struct node_ref {
  node_ref() = default;
  node_ref(geo::latlng coords, size_t id) : coords_(coords), id_(id) {}

  geo::latlng coords_;
  size_t id_;
};

struct routing_result {
  routing_result(geo::polyline polyline, double weight)
      : polyline_(std::move(polyline)), weight_(weight) {}
  geo::polyline polyline_;
  double weight_;
};

struct routing_strategy {

  virtual std::vector<node_ref> close_nodes(std::string const& station_id) = 0;
  virtual std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from, std::vector<node_ref> const& to) = 0;
};

struct stub_routing : routing_strategy {

  stub_routing(station_seq const& seq) : seq_(seq){};

  virtual std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from, std::vector<node_ref> const& to) {

    std::vector<std::vector<routing_result>> result;

    for (auto const& f : from) {

      std::vector<routing_result> from_result;

      for (auto const& t : to) {
        from_result.emplace_back(geo::polyline{f.coords_, t.coords_},
                                 distance(f.coords_, t.coords_));
      }
      result.emplace_back(std::move(from_result));
    }
    return result;
  }

  virtual std::vector<node_ref> close_nodes(std::string const& station_id) {
    auto it =
        std::find(begin(seq_.station_ids_), end(seq_.station_ids_), station_id);
    if (it == end(seq_.station_ids_)) {
      return {};
    }
    std::vector<node_ref> result;
    node_ref ref;
    ref.coords_ =
        seq_.coordinates_[std::distance(begin(seq_.station_ids_), it)];
    ref.id_ = -1;
    result.push_back(ref);
    return result;
  }

  station_seq const& seq_;
};

}  // routes
}  // motis
