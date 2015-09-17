#pragma once

#include "catch/catch.hpp"

#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/schedule.h"

#include "motis/reliability/graph_accessor.h"

namespace motis {
namespace reliability {

struct interchange_data_for_tests {
  interchange_data_for_tests(
      schedule const& schedule, unsigned int const arriving_train_route_idx,
      unsigned int const departing_train_route_idx,
      unsigned int const previous_station,
      unsigned int const interchange_station, unsigned int const next_station,
      unsigned int arriving_train_dep, unsigned int arriving_train_arr,
      unsigned int departing_train_dep, unsigned int departing_train_arr)
      : interchange_data_for_tests(schedule, arriving_train_route_idx,
                         *schedule.route_index_to_first_route_node.at(
                             departing_train_route_idx),
                         previous_station, interchange_station, next_station,
                         arriving_train_dep, arriving_train_arr,
                         departing_train_dep, departing_train_arr) {}
  interchange_data_for_tests(
      schedule const& schedule, unsigned int const arriving_train_route_idx,
      node const& tail_node_departing_train,
      unsigned int const previous_station,
      unsigned int const interchange_station, unsigned int const next_station,
      unsigned int arriving_train_dep, unsigned int arriving_train_arr,
      unsigned int departing_train_dep, unsigned int departing_train_arr)
      : arriving_route_edge_(*graph_accessor::get_departing_route_edge(
                                 *schedule.route_index_to_first_route_node.at(
                                     arriving_train_route_idx))),
        arriving_light_conn_(arriving_route_edge_._m._route_edge._conns[0]),
        tail_node_departing_train_(tail_node_departing_train),
        departing_route_edge_(*graph_accessor::get_departing_route_edge(
                                  tail_node_departing_train_)),
        departing_light_conn_(departing_route_edge_._m._route_edge._conns[0]) {
    REQUIRE(tail_node_departing_train_._station_node->_id ==
            interchange_station);
    REQUIRE(arriving_route_edge_._from->_station_node->_id == previous_station);
    REQUIRE(arriving_route_edge_._to->_station_node->_id ==
            interchange_station);
    REQUIRE(departing_route_edge_._from->_station_node->_id ==
            interchange_station);
    REQUIRE(departing_route_edge_._to->_station_node->_id == next_station);

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
