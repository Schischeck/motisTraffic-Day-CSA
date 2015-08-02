#pragma once

#include <map>
#include <vector>

#include "boost/filesystem.hpp"

#include "motis/core/schedule/date_manager.h"
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

  date_manager date_mgr;
  std::vector<station_ptr> stations;
  std::map<int, station*> eva_to_station;
  std::vector<std::string> category_names;
  std::map<std::string, int> classes;
  std::map<int, std::string> tracks;
  std::map<int, attribute> attributes;
  constant_graph lower_bounds;
  unsigned node_count;
  std::vector<station_node_ptr> station_nodes;
  std::vector<node*> route_index_to_first_route_node;
  waiting_time_rules waiting_time_rules_;
  std::vector<std::unique_ptr<connection>> full_connections;
  std::vector<std::unique_ptr<connection_info>> connection_infos;
  synchronization sync;
};

typedef std::unique_ptr<schedule> schedule_ptr;

}  // namespace motis
