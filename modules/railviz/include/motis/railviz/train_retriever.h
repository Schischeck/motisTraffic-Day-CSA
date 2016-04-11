#pragma once

#include <climits>
#include <memory>
#include <vector>

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

class edge_geo_index;

struct train_retriever {
  train_retriever(schedule const& s);
  ~train_retriever();

  std::vector<std::pair<light_connection const*, edge const*>> trains(
      const time from, const time to, unsigned max_count, geo::box area);
  std::pair<light_connection const*, edge const*> search_train(
      std::string train_number, const time from, const time to,
      unsigned classz);

  std::vector<std::unique_ptr<edge_geo_index>> edge_index_;
  schedule const& schedule_;
};

}  // namespace railviz
}  // namespace motis
