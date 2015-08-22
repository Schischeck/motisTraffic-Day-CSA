#pragma once

#include <memory>
#include <vector>
#include <climits>

#include <utility>  // std::pair, std::make_pair

#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/station.h"
#include "motis/core/schedule/time.h"
#include "motis/railviz/geo.h"

namespace motis {

class edge;
struct schedule;
struct light_connection;

namespace railviz {

struct edge_geo_index;

struct train_retriever {
  train_retriever(schedule const& s);
  ~train_retriever();

  std::vector<std::pair<light_connection const*, edge const*>> trains(
      const time from, const time to, int max_count, geo::box area);

  std::vector<std::unique_ptr<edge_geo_index>> edge_index_;
  schedule const& schedule_;
};

}  // namespace railviz
}  // namespace motis
