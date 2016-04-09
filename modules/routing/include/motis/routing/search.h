#pragma once

#include <memory>
#include <string>
#include <vector>

#include "motis/core/schedule/time.h"
#include "motis/core/journey/journey.h"

#include "motis/routing/arrival.h"
#include "motis/routing/label/comparator.h"
#include "motis/routing/label/criteria/transfers.h"
#include "motis/routing/label/criteria/travel_time.h"
#include "motis/routing/label/dominance.h"
#include "motis/routing/label/filter.h"
#include "motis/routing/label/initializer.h"
#include "motis/routing/label/label.h"
#include "motis/routing/label/tie_breakers.h"
#include "motis/routing/label/updater.h"
#include "motis/routing/lower_bounds.h"
#include "motis/routing/pareto_dijkstra.h"
#include "motis/routing/statistics.h"

namespace motis {

struct schedule;

namespace routing {

struct search_result {
  search_result() = default;
  search_result(statistics stats, std::vector<journey> journeys)
      : stats(stats), journeys(std::move(journeys)) {}
  statistics stats;
  std::vector<journey> journeys;
};

typedef label<
    label_data<travel_time, transfers>,
    initializer<travel_time_initializer, transfers_initializer>,
    updater<travel_time_updater, transfers_updater>,
    filter<travel_time_filter, transfers_filter, waiting_time_filter>,
    dominance<default_tb, travel_time_dominance, transfers_dominance>,
    dominance<post_search_tb, travel_time_alpha_dominance, transfers_dominance>,
    comparator<travel_time_dominance, transfers_dominance>>
    my_label;

class search {
public:
  search(schedule const& schedule, memory_manager& label_store);

  search_result get_connections(arrival from, arrival to, time interval_start,
                                time interval_end, bool ontrip,
                                std::vector<edge> const& additional_edges);

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
