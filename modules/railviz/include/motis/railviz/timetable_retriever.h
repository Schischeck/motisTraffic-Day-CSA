#pragma once

#include <algorithm>
#include <iostream>
#include <set>
#include <unordered_map>
#include <utility>

#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/schedule.h"

namespace motis {
namespace railviz {

typedef std::tuple<light_connection const*, station_node const*,
                   station_node const*, station_node const*, bool, unsigned int>
    timetable_entry;
typedef std::vector<timetable_entry> timetable;

typedef std::tuple<station_node const*, motis::node const*,
                   light_connection const*>
    route_entry;
typedef std::vector<route_entry> route;

struct timetable_retriever {
  struct timetable_sort_struct {
    bool operator()(const timetable_entry& e1, const timetable_entry& e2) {
      motis::time t1 =
          (std::get<3>(e1)) ? std::get<0>(e1)->d_time : std::get<0>(e1)->a_time;
      motis::time t2 =
          (std::get<3>(e2)) ? std::get<0>(e2)->d_time : std::get<0>(e2)->a_time;
      return t1 < t2;
    }
  } timetable_sort;

  std::vector<motis::station_node const*> stations_on_route(
      const motis::node&) const;
  std::vector<route> get_routes_on_time(const motis::node&,
                                        motis::time time) const;

  timetable ordered_timetable_for_station(const station_node& station) const;

  void timetable_for_station_outgoing(const station_node& station,
                                      timetable& timetable_) const;
  void timetable_for_station_incoming(const station_node& station,
                                      timetable& timetable_) const;

  const motis::node* parent_node(const motis::node& node) const;
  const motis::node* child_node(const motis::node& node) const;

  const motis::node* start_node_for_route(const motis::node&) const;
  const motis::node* end_node_for_route(const motis::node&) const;

  std::vector<motis::time> get_route_departure_times(const motis::node&) const;

  // std::map<unsigned int, motis::node const*> route_start_node;
  // std::map<unsigned int, motis::node const*> route_end_node;
};
}
}
