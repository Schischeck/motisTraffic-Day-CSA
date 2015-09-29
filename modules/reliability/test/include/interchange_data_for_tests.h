#pragma once

#include <string>

#include "catch/catch.hpp"

#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/schedule.h"

#include "motis/reliability/graph_accessor.h"

namespace motis {
namespace reliability {

struct interchange_data_for_tests {
  interchange_data_for_tests(schedule const& schedule,
                             unsigned int const arriving_train_id,
                             unsigned int const departing_train_id,
                             std::string const& previous_station_eva,
                             std::string const& interchange_station_eva,
                             std::string const& next_station_eva,
                             unsigned int arriving_train_dep,
                             unsigned int arriving_train_arr,
                             unsigned int departing_train_dep,
                             unsigned int departing_train_arr)
      : interchange_data_for_tests(
            schedule, arriving_train_id,
            *graph_accessor::get_first_route_node(schedule, departing_train_id),
            previous_station_eva, interchange_station_eva, next_station_eva,
            arriving_train_dep, arriving_train_arr, departing_train_dep,
            departing_train_arr) {}
  interchange_data_for_tests(schedule const& schedule,
                             unsigned int const arriving_train_id,
                             node const& tail_node_departing_train,
                             std::string const& previous_station_eva,
                             std::string const& interchange_station_eva,
                             std::string const& next_station_eva,
                             unsigned int arriving_train_dep,
                             unsigned int arriving_train_arr,
                             unsigned int departing_train_dep,
                             unsigned int departing_train_arr)
      : arriving_route_edge_(*graph_accessor::get_departing_route_edge(
                                 *graph_accessor::get_first_route_node(
                                     schedule, arriving_train_id))),
        arriving_light_conn_(arriving_route_edge_._m._route_edge._conns[0]),
        tail_node_departing_train_(tail_node_departing_train),
        departing_route_edge_(*graph_accessor::get_departing_route_edge(
                                  tail_node_departing_train_)),
        departing_light_conn_(departing_route_edge_._m._route_edge._conns[0]) {
    REQUIRE(schedule.stations[tail_node_departing_train_._station_node->_id]
                ->eva_nr == interchange_station_eva);
    REQUIRE(schedule.stations[arriving_route_edge_._from->_station_node->_id]
                ->eva_nr == previous_station_eva);
    REQUIRE(schedule.stations[arriving_route_edge_._to->_station_node->_id]
                ->eva_nr == interchange_station_eva);
    REQUIRE(schedule.stations[departing_route_edge_._from->_station_node->_id]
                ->eva_nr == interchange_station_eva);
    REQUIRE(schedule.stations[departing_route_edge_._to->_station_node->_id]
                ->eva_nr == next_station_eva);

    REQUIRE(arriving_light_conn_.d_time == (motis::time)arriving_train_dep);
    REQUIRE(arriving_light_conn_.a_time == (motis::time)arriving_train_arr);
    REQUIRE(departing_light_conn_.d_time == (motis::time)departing_train_dep);
    REQUIRE(departing_light_conn_.a_time == (motis::time)departing_train_arr);
  }

  node const& tail_node_departing_train_;
  edge const& arriving_route_edge_;
  light_connection const& arriving_light_conn_;
  edge const& departing_route_edge_;
  light_connection const& departing_light_conn_;
};
}
}
