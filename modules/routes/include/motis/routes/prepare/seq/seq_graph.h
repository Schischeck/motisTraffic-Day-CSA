#pragma once

#include <vector>
#include <memory>

#include "motis/geo/latlng.h"
#include "motis/geo/polyline.h"

namespace motis {
namespace routes {

struct poly_edge;

struct station_p {

  station_p(size_t idx, size_t station, size_t match, geo::latlng coords)
      : station_(station), match_(match), coords_(coords), idx_(idx){};

  std::vector<poly_edge> edges_;
  size_t station_;
  long match_;
  geo::latlng coords_;
  size_t idx_;
};

struct poly_edge {

  poly_edge(station_p* from, station_p* to, geo::polyline p, int offset,
            int length, float weight)
      : from_(from),
        to_(to),
        p_(p),
        offset_(offset),
        length_(length),
        weight_(weight){};

  station_p* from_;
  station_p* to_;
  geo::polyline p_;
  int offset_;
  int length_;
  float weight_;
};

struct seq_graph {
  std::vector<std::unique_ptr<station_p>> nodes_;
  std::vector<std::vector<station_p*>> station_to_nodes_;
};

}  // namespace routes
}  // namespace motis
