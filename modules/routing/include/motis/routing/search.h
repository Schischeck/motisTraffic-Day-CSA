#pragma once

#include <vector>
#include <string>
#include <memory>

#include "motis/core/journey/journey.h"
#include "motis/core/schedule/time.h"

#include "motis/routing/lower_bounds.h"
#include "motis/routing/pareto_dijkstra.h"
#include "motis/routing/arrival.h"

#include "motis/routing/label/label.h"
#include "motis/routing/label/initializer.h"
#include "motis/routing/label/updater.h"
#include "motis/routing/label/comparator.h"
#include "motis/routing/label/dominance.h"
#include "motis/routing/label/criteria/transfers.h"
#include "motis/routing/label/criteria/travel_time.h"

namespace motis {

struct schedule;

namespace routing {

typedef label<label_data<travel_time, transfers>,
              initializer<travel_time_initializer, transfers_initializer>,
              updater<travel_time_updater, transfers_updater>,
              dominance<travel_time_dominance, transfers_dominance>,
              comparator<travel_time_dominance, transfers_dominance>> my_label;

struct statistics;

class search {
public:
  search(schedule const& schedule, memory_manager& label_store);

  std::vector<journey> get_connections(arrival from, arrival to,
                                       time interval_start, time interval_end,
                                       bool ontrip);

  void generate_ontrip_start_labels(station_node const* start_station,
                                    time const start_time,
                                    std::vector<my_label*>& start_labels,
                                    lower_bounds& context);

  void generate_start_labels(time const from, time const to,
                             station_node const* start_station_node,
                             std::vector<my_label*>& indices,
                             station_node const* real_start, int time_off,
                             lower_bounds& lower_bounds);

  void generate_start_labels(time const from, time const to,
                             station_node const* start_station_node,
                             node const* route_node,
                             std::vector<my_label*>& start_labels,
                             station_node const* real_start, int time_off,
                             lower_bounds&);

  schedule const& _sched;
  memory_manager& _label_store;
};

}  // namespace routing
}  // namespace motis
