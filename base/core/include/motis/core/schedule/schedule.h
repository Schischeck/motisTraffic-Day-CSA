#pragma once

#include <ctime>
#include <map>
#include <unordered_map>
#include <vector>

#include "motis/core/common/hash_map.h"
#include "motis/core/common/synchronization.h"
#include "motis/core/schedule/attribute.h"
#include "motis/core/schedule/category.h"
#include "motis/core/schedule/constant_graph.h"
#include "motis/core/schedule/delay_info.h"
#include "motis/core/schedule/event.h"
#include "motis/core/schedule/event.h"
#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/provider.h"
#include "motis/core/schedule/station.h"
#include "motis/core/schedule/trip.h"
#include "motis/core/schedule/waiting_time_rules.h"

namespace motis {

struct schedule {
  schedule()
      : schedule_begin_(0),
        schedule_end_(0),
        node_count_(0),
        system_time_(0),
        last_update_timestamp_(0) {
    graph_to_delay_info_.set_empty_key({nullptr, 0, event_type::DEP});
  }

  schedule(schedule const&) = delete;
  schedule& operator=(schedule const&) = delete;

  std::time_t schedule_begin_, schedule_end_;
  std::vector<station_ptr> stations_;
  std::map<std::string, station*> eva_to_station_;
  std::map<std::string, station*> ds100_to_station_;
  std::map<std::string, int> classes_;
  std::vector<std::string> tracks_;
  constant_graph lower_bounds_;
  unsigned node_count_;
  std::vector<station_node_ptr> station_nodes_;
  std::vector<node*> route_index_to_first_route_node_;
  std::unordered_map<uint32_t, std::vector<int32_t>> train_nr_to_routes_;
  waiting_time_rules waiting_time_rules_;
  synchronization sync_;

  std::vector<std::unique_ptr<connection>> full_connections_;
  std::vector<std::unique_ptr<connection_info>> connection_infos_;
  std::vector<std::unique_ptr<attribute>> attributes_;
  std::vector<std::unique_ptr<category>> categories_;
  std::vector<std::unique_ptr<provider>> providers_;
  std::vector<std::unique_ptr<std::string>> directions_;
  std::vector<std::unique_ptr<timezone>> timezones_;

  std::vector<std::pair<primary_trip_id, trip*>> trips_;
  std::vector<std::unique_ptr<trip>> trip_mem_;
  std::vector<std::unique_ptr<std::vector<trip::route_edge>>> trip_edges_;
  std::vector<std::unique_ptr<std::vector<trip*>>> merged_trips_;

  std::time_t system_time_, last_update_timestamp_;
  std::vector<std::unique_ptr<delay_info>> delay_mem_;
  hash_map<ev_key, delay_info*> graph_to_delay_info_;
};

typedef std::unique_ptr<schedule> schedule_ptr;

}  // namespace motis
