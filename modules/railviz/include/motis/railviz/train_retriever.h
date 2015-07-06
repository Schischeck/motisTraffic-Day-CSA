#pragma once

#include <vector>
#include <climits>

#include "motis/core/schedule/time.h"

#include "motis/railviz/geo.h"

namespace motis {

struct schedule;
struct light_connection;

namespace railviz {

struct edge_geo_index;
struct train
{
    std::time_t d_time;
    std::time_t a_time;
    unsigned int d_station;
    unsigned int a_station;
};

typedef std::unique_ptr<train> train_ptr;
typedef std::unique_ptr<std::vector<train_ptr>> train_list_ptr;

struct train_retriever {
  train_retriever(schedule const& s);
  ~train_retriever();

  train_list_ptr trains(const time from, const time to,
                                			geo::box area, int max_count=INT_MAX);
	
  std::vector<std::unique_ptr<edge_geo_index>> edge_index_;
};

}  // namespace railviz
}  // namespace motis
