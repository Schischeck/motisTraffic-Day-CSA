#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include <map>
#include <set>
#include <map>

#include "motis/core/schedule/attribute.h"
#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/station.h"

namespace motis {

class bitset_manager;
class date_manager;
class node;
class edge;
class station_node;
class connection;
class input_connection;
class light_connection;
class connection_info;
class waiting_time_rules;

class graph_loader {
public:
  graph_loader(const std::string& prefix);

  void load_dates(date_manager& dm);
  int load_stations(std::vector<station_ptr>& stations,
                    std::vector<station_node_ptr>& station_nodes);
  void load_bitfields(bitset_manager& bm);
  int load_routes(
      int node_id, std::vector<station_ptr>& stations,
      std::vector<station_node_ptr> const& station_nodes,
      const std::map<int, int>& class_mapping,
      std::vector<std::unique_ptr<connection>>& full_connections,
      std::vector<std::unique_ptr<connection_info>>& connection_infos,
      std::vector<node*>& route_index_to_first_route_node);
  void assign_predecessors(std::vector<station_node_ptr>& station_nodes);
  int load_foot_paths(int node_id,
                      std::vector<station_node_ptr> const& station_nodes);
  void load_attributes(std::map<int, attribute>& attributes_map);
  void load_classes(std::map<std::string, int>& map);
  void load_category_names(std::vector<std::string>& category_names);
  void load_tracks(std::map<int, std::string>& track_map);
  void load_waiting_time_rules(std::vector<std::string> const& category_names,
                               waiting_time_rules& rules);

private:
  /** bitfield rollout (= absolute times, relative to day 0 of schedule) */
  static std::vector<light_connection> expand_bitfields(
      connection const* full_con, input_connection const& con,
      bitset_manager const& bm);

  /** @return the distance between the given stations. */
  double get_distance(const station& s1, const station& s2);

  // prefix of schedule files
  std::string _prefix;
};

}  // namespace motis
