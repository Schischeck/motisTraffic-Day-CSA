#pragma once

#include <vector>

#include "motis/core/common/geo.h"

#include "motis/routes/prepare/rel/relation_matcher.h"
#include "motis/routes/prepare/station_sequences.h"

namespace motis {
namespace routes {

struct poly_edge;
struct station_p {

  station_p(size_t station, size_t match, latlng coords)
      : station_(station), match_(match), coords_(coords){};

  std::vector<poly_edge> edges_;
  size_t station_;
  long match_;
  latlng coords_;
};

struct poly_edge {

  poly_edge(station_p* from, station_p* to, std::vector<latlng> p, int offset,
            int length, float weight)
      : from_(from),
        to_(to),
        p_(p),
        offset_(offset),
        length_(length),
        weight_(weight){};

  station_p* from_;
  station_p* to_;
  std::vector<latlng> p_;
  int offset_;
  int length_;
  float weight_;
};

struct graph {
  std::vector<std::unique_ptr<station_p>> nodes_;
  std::vector<std::vector<station_p*>> station_to_nodes_;
};

void connect_matches(std::vector<station_seq> const& sequences,
                     std::vector<std::vector<match_seq>>& matches);

void build_graph(station_seq const& seq, std::vector<match_seq>& matches);

void create_nodes(graph& g, std::vector<match_seq>& matches);

void create_unmatched_nodes(graph& g, station_seq const& seq);

void create_edges(graph& g);

void connect_nodes(std::vector<station_p*>& station1,
                   std::vector<station_p*>& station2);

float calc_weight(std::vector<latlng> const& polyline);

std::vector<latlng> find_route(latlng const& p1, latlng const& p2);

}  // namespace routes
}  // namespace motis
