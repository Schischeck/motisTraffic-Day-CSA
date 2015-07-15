#pragma once

#include <vector>
#include <climits>

#include <utility>  // std::pair, std::make_pair

#include "motis/core/schedule/time.h"

#include "motis/railviz/geo.h"

namespace motis {

struct edge;
struct schedule;
struct light_connection;

namespace railviz {

struct edge_geo_index;

struct train_retriever {
  train_retriever(schedule const& s);
  ~train_retriever();

  typedef std::pair<edge const*, light_connection const*> train_pair;
  typedef std::vector<train_pair> train_vector;

  train_vector trains(const time from, const time to,
                                                        geo::box area, int max_count=INT_MAX);

  std::vector<std::unique_ptr<edge_geo_index>> edge_index_;
};

}  // namespace railviz
}  // namespace motis
