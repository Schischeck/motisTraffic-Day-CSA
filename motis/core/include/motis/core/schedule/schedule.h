#pragma once

#include <ctime>
#include <map>
#include <unordered_map>
#include <vector>

#include "motis/core/schedule/station.h"
#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/constant_graph.h"
#include "motis/core/schedule/attribute.h"
#include "motis/core/schedule/category.h"
#include "motis/core/schedule/provider.h"
#include "motis/core/schedule/waiting_time_rules.h"
#include "motis/core/common/synchronization.h"

namespace motis {

class connection;
class connection_info;

struct schedule {
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
};

typedef std::unique_ptr<schedule> schedule_ptr;

}  // namespace motis
