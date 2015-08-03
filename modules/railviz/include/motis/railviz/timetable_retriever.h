#pragma once

#include <iostream>
#include <set>
#include <unordered_map>
#include <utility>
#include <algorithm>

#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/connection.h"

namespace motis {
namespace railviz {

typedef std::tuple<light_connection const*, station_node const*,
                   station_node const*, bool, unsigned int> timetable_entry;
typedef std::vector<timetable_entry> timetable;

typedef std::pair<station_node const*, light_connection const*> route_entry;
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

  void init(schedule const& sched);

  std::vector<route> get_routes_on_time(unsigned int route_id,
                                        motis::time time) const;
  const light_connection* get_track_information(const station_node& station,
                                                unsigned int route_id,
                                                int track) const;

  timetable ordered_timetable_for_station(const station_node& station) const;

  void timetable_for_station_outgoing(const station_node& station,
                                      timetable& timetable_) const;
  void timetable_for_station_incoming(const station_node& station,
                                      timetable& timetable_) const;

  const motis::station_node* next_station_on_route(
      const motis::node& route_node) const;
  const motis::station_node* next_station_on_route(
      const motis::station_node& node, unsigned int route_id) const;

  const motis::station_node* end_station_for_route(
      unsigned int route_id, const node* current_node) const;
  const motis::station_node* end_station_for_route(
      unsigned int route_id, const node* current_node,
      std::set<unsigned int>&) const;

  const motis::station_node* prev_station_on_route(
      const motis::node& route_node) const;
  const motis::station_node* prev_station_on_route(
      const motis::station_node& node, unsigned int route_id) const;

  const motis::station_node* start_station_for_route(
      unsigned int route_id, const node* current_node) const;
  const motis::station_node* start_station_for_route(
      unsigned int route_id, const node* current_node,
      std::set<unsigned int>&) const;

  std::vector<unsigned int> routes_at_station(const station_node&) const;
  std::vector<motis::time> get_route_departure_times(
      unsigned int route_id) const;
  std::vector<motis::time> get_route_arrival_times(unsigned int route_id) const;

  std::map<unsigned int, station_node const*> route_start_station;
  std::map<unsigned int, station_node const*> route_end_station;
};
}
}
