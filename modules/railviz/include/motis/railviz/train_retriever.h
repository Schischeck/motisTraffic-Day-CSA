#pragma once

#include <memory>
#include <vector>
#include <climits>

#include <utility>  // std::pair, std::make_pair

#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/station.h"
#include "motis/core/schedule/time.h"
#include "motis/railviz/train.h"
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

  std::vector<train> trains(const time from, const time to,
                       geo::box area, int max_count=INT_MAX);

  std::vector<train> timetable_for_station_outgoing( const station_node& station );

  geo::box bounds();

  const motis::station_node* end_station_for_route( unsigned int route_id ) const;
  const motis::station_node* end_station_for_route( unsigned int route_id, const node* current_node ) const;

  std::vector<std::unique_ptr<edge_geo_index>> edge_index_;
  schedule const& schedule_;
};

}  // namespace railviz
}  // namespace motis
