#pragma once

#include <vector>
#include <string>
#include <memory>

#include "motis/core/journey/journey.h"
#include "motis/core/schedule/time.h"

#include "motis/routing/lower_bounds.h"
#include "motis/routing/pareto_dijkstra.h"
#include "motis/routing/arrival.h"
#include "motis/routing/label.h"

namespace motis {

struct schedule;

struct search_result {
  search_result() = default;
  search_result(pareto_dijkstra::statistics stats,
                std::vector<journey> journeys)
      : stats(stats), journeys(std::move(journeys)) {}
  pareto_dijkstra::statistics stats;
  std::vector<journey> journeys;
};

class search {
public:
  search(schedule const& schedule, memory_manager<label>& label_store);

  search_result get_connections(arrival from, arrival to, time interval_start,
                                time interval_end, bool ontrip);

  void generate_ontrip_start_labels(station_node const* start_station,
                                    time const start_time,
                                    std::vector<label*>& start_labels,
                                    lower_bounds& context);

  void generate_start_labels(time const from, time const to,
                             station_node const* start_station_node,
                             std::vector<label*>& start_labels,
                             station_node const* real_start, int time_off,
                             int start_price, int slot, lower_bounds& context);

  void generate_start_labels(time const from, time const to,
                             station_node const* start_station_node,
                             node const* route,
                             std::vector<label*>& start_labels,
                             station_node const* real_start, int time_off,
                             int start_price, int slot, lower_bounds& context);

  schedule const& _sched;
  memory_manager<label>& _label_store;
};

}  // namespace motis
