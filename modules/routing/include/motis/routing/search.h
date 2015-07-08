#pragma once

#include <vector>
#include <string>
#include <memory>

#include "motis/routing/lower_bounds.h"
#include "motis/routing/pareto_dijkstra.h"
#include "motis/routing/arrival.h"
#include "motis/routing/label.h"
#include "motis/routing/journey.h"
#include "motis/core/schedule/time.h"

namespace motis {

struct schedule;

class search {
public:
  search(schedule& schedule, memory_manager<label>& label_store);

  std::vector<journey> get_connections(
      arrival from, arrival to, time interval_start, time interval_end,
      pareto_dijkstra::statistics* stats = nullptr);

  void generate_start_labels(time const from, time const to,
                             station_node const* start_station_node,
                             std::vector<label*>& indices,
                             station_node const* real_start, int time_off,
                             int start_price, int slot, lower_bounds& context);

  void generate_start_labels(time const from, time const to,
                             station_node const* start_station_node,
                             node const* route, std::vector<label*>& indices,
                             station_node const* real_start, int time_off,
                             int start_price, int slot, lower_bounds& context);

  schedule& _sched;
  memory_manager<label>& _label_store;
};

}  // namespace motis
