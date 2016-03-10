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
#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/provider.h"
#include "motis/core/schedule/station.h"
#include "motis/core/schedule/waiting_time_rules.h"
#include "motis/core/schedule/trip.h"

namespace motis {

struct connection;
struct connection_info;

struct schedule {
  schedule() {
    constexpr auto i = std::numeric_limits<int32_t>::max();
    constexpr auto iu = std::numeric_limits<uint32_t>::max();

    schedule_to_delay_info.set_empty_key({iu, iu, true, INVALID_TIME});
    graph_to_delay_info.set_empty_key({iu, iu, true, INVALID_TIME, i});
    trips.set_empty_key(primary_trip_id(0, 0, 0));
    graph_to_delay_info.set_deleted_key({iu - 1, iu, true, INVALID_TIME, i});
  }
  virtual ~schedule() {}

  std::time_t schedule_begin_, schedule_end_;
  std::vector<station_ptr> stations;
  std::map<std::string, station*> eva_to_station;
  std::map<std::string, station*> ds100_to_station;
  std::map<std::string, int> classes;
  std::vector<std::string> tracks;
  constant_graph lower_bounds;
  unsigned node_count;
  std::vector<station_node_ptr> station_nodes;
  std::vector<node*> route_index_to_first_route_node;
  std::unordered_map<uint32_t, std::vector<int32_t>> train_nr_to_routes;
  waiting_time_rules waiting_time_rules_;
  synchronization sync;

  std::vector<std::unique_ptr<connection>> full_connections;
  std::vector<std::unique_ptr<connection_info>> connection_infos;
  std::vector<std::unique_ptr<attribute>> attributes;
  std::vector<std::unique_ptr<category>> categories;
  std::vector<std::unique_ptr<provider>> providers;
  std::vector<std::unique_ptr<std::string>> directions;
  std::vector<std::unique_ptr<timezone>> timezones;

  hash_map<primary_trip_id, trip*> trips;
  std::vector<std::unique_ptr<trip>> trips_mem;

  std::vector<std::unique_ptr<delay_info>> delay_infos;
  hash_map<schedule_event, delay_info*> schedule_to_delay_info;
  hash_map<graph_event, delay_info*> graph_to_delay_info;
};

typedef std::unique_ptr<schedule> schedule_ptr;

}  // namespace motis
