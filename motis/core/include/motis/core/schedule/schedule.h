#pragma once

#include <ctime>
#include <map>
#include <vector>

#include "motis/core/schedule/station.h"
#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/constant_graph.h"
#include "motis/core/schedule/attribute.h"
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
  std::vector<std::string> category_names;
  std::map<std::string, int> classes;
  std::vector<std::string> tracks;
  constant_graph lower_bounds;
  unsigned node_count;
  std::vector<station_node_ptr> station_nodes;
  std::vector<node*> route_index_to_first_route_node;
  waiting_time_rules waiting_time_rules_;
  synchronization sync;

  std::vector<std::unique_ptr<connection>> full_connections;
  std::vector<std::unique_ptr<connection_info>> connection_infos;
  std::vector<std::unique_ptr<attribute>> attributes;
};

typedef std::unique_ptr<schedule> schedule_ptr;

}  // namespace motis
