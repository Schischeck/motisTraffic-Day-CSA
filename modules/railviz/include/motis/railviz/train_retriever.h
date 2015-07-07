#pragma once

#include <vector>

#include "motis/core/schedule/time.h"

#include "motis/railviz/geo.h"

namespace motis {

struct schedule;
struct light_connection;

namespace railviz {

struct edge_geo_index;

struct train_retriever {
  train_retriever(schedule const& s);
  ~train_retriever();

  std::vector<light_connection const*> connections(const time from,
                                                   const time to, int max_count,
                                                   geo::box area);

  std::vector<std::unique_ptr<edge_geo_index>> edge_index_;
};

}  // namespace railviz
}  // namespace motis