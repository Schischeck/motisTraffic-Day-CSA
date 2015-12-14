#include "gtest/gtest.h"

#include <iostream>

#include "motis/loader/loader.h"

#include "motis/core/common/date_util.h"

#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

#include "motis/reliability/graph_accessor.h"

#include "include/test_schedule_setup.h"
#include "include/test_util.h"

namespace motis {
namespace reliability {
namespace graph_accessor {

class reliability_graph_accessor : public test_schedule_setup {
public:
  reliability_graph_accessor()
      : test_schedule_setup("modules/reliability/resources/schedule/",
                            "20150928") {}
  /* eva numbers */
  std::string const DARMSTADT = "4219971";
  std::string const FRANKFURT = "8351230";
  std::string const HEIDELBERG = "9335048";
  /* train numbers */
  short const IC_DA_H = 1;
  short const IC_FR_DA = 2;
  short const IC_FH_DA = 3;
  short const RE_MA_DA = 4;
  short const ICE_FR_DA_H = 5;
  short const ICE_HA_W_HE = 6;
  short const ICE_K_K = 7;
  short const RE_K_S = 8;
};

class reliability_graph_accessor3 : public test_schedule_setup {
public:
  reliability_graph_accessor3()
      : test_schedule_setup("modules/reliability/resources/schedule3/",
                            "20150928") {}
  short const ICE_L_H = 1;  // 10:00 --> 10:10
  short const S_M_W = 2;  // 10:20 --> 10:25
};

TEST_F(reliability_graph_accessor, get_first_route_node_by_train_nr) {
  auto node = get_first_route_node(*schedule_, ICE_FR_DA_H);
  ASSERT_EQ(schedule_->stations[node->_station_node->_id]->eva_nr, FRANKFURT);
  ASSERT_EQ(get_departing_route_edge(*node)
                ->_m._route_edge._conns[0]
                ._full_con->con_info->train_nr,
            ICE_FR_DA_H);
}

TEST_F(reliability_graph_accessor, get_previous_light_connection) {
  auto const first_route_node = get_first_route_node(*schedule_, ICE_FR_DA_H);
  ASSERT_EQ(schedule_->stations[first_route_node->_station_node->_id]->eva_nr,
            FRANKFURT);
  auto const first_route_edge = get_departing_route_edge(*first_route_node);
  {
    auto const& first_light_conn = first_route_edge->_m._route_edge._conns[0];
    ASSERT_EQ(first_light_conn.d_time,
              test_util::minutes_to_motis_time(5 * 60 + 55));

    auto second_route_node = first_route_edge->_to;
    ASSERT_EQ(
        schedule_->stations[second_route_node->_station_node->_id]->eva_nr,
        DARMSTADT);
    ASSERT_EQ(get_arriving_route_edge(*second_route_node), first_route_edge);

    auto const& second_light_conn =
        get_departing_route_edge(*second_route_node)->_m._route_edge._conns[0];
    ASSERT_EQ(second_light_conn.d_time,
              test_util::minutes_to_motis_time(6 * 60 + 11));
    auto const& previous_light_conn = get_previous_light_connection(
        graph_accessor::get_arriving_route_edge(*second_route_node)
            ->_m._route_edge._conns,
        second_light_conn.d_time);

    ASSERT_EQ(&previous_light_conn, &first_light_conn);
  }
  {
    auto const& first_light_conn = first_route_edge->_m._route_edge._conns[1];
    ASSERT_EQ(first_light_conn.d_time,
              test_util::minutes_to_motis_time(6 * 60 + 55));

    auto second_route_node = first_route_edge->_to;
    ASSERT_EQ(
        schedule_->stations[second_route_node->_station_node->_id]->eva_nr,
        DARMSTADT);
    ASSERT_EQ(get_arriving_route_edge(*second_route_node), first_route_edge);

    auto const& second_light_conn =
        get_departing_route_edge(*second_route_node)->_m._route_edge._conns[1];
    ASSERT_EQ(second_light_conn.d_time,
              test_util::minutes_to_motis_time(7 * 60 + 11));
    auto const& previous_light_conn = get_previous_light_connection(
        graph_accessor::get_arriving_route_edge(*second_route_node)
            ->_m._route_edge._conns,
        second_light_conn.d_time);

    ASSERT_EQ(&previous_light_conn, &first_light_conn);
  }
}

TEST_F(reliability_graph_accessor, get_previous_light_connection2) {
  array<light_connection> arriving_connections;
  arriving_connections.emplace_back(10, 20, nullptr);
  arriving_connections.emplace_back(15, 25, nullptr);
  arriving_connections.emplace_back(15, 30, nullptr);
  arriving_connections.emplace_back(20, 30, nullptr);
  arriving_connections.emplace_back(40, 50, nullptr);

  ASSERT_EQ(
      &arriving_connections[0],
      &graph_accessor::get_previous_light_connection(arriving_connections, 20));
  ASSERT_EQ(
      &arriving_connections[0],
      &graph_accessor::get_previous_light_connection(arriving_connections, 24));
  ASSERT_EQ(
      &arriving_connections[1],
      &graph_accessor::get_previous_light_connection(arriving_connections, 25));
  ASSERT_EQ(
      &arriving_connections[1],
      &graph_accessor::get_previous_light_connection(arriving_connections, 29));
  ASSERT_EQ(
      &arriving_connections[3],
      &graph_accessor::get_previous_light_connection(arriving_connections, 30));
  ASSERT_EQ(
      &arriving_connections[3],
      &graph_accessor::get_previous_light_connection(arriving_connections, 31));
  ASSERT_EQ(
      &arriving_connections[4],
      &graph_accessor::get_previous_light_connection(arriving_connections, 50));
  ASSERT_EQ(
      &arriving_connections[4],
      &graph_accessor::get_previous_light_connection(arriving_connections, 51));
}

TEST_F(reliability_graph_accessor, get_first_route_node) {
  auto const first_node = get_first_route_node(*schedule_, ICE_FR_DA_H);
  auto const node =
      get_departing_route_edge(*get_departing_route_edge(*first_node)->_to)
          ->_to;
  ASSERT_EQ(&get_first_route_node(*node), first_node);
}

TEST_F(reliability_graph_accessor, get_light_connection) {
  edge route_edge;
  route_edge._m._type = edge::ROUTE_EDGE;

  light_connection lc1(10);
  connection fc1;
  connection_info ci1;
  ci1.train_nr = 1;
  ci1.family = 1;
  fc1.con_info = &ci1;
  lc1._full_con = &fc1;
  route_edge._m._route_edge._conns.push_back(lc1);

  light_connection lc2(20);
  connection fc2;
  connection_info ci2;
  ci2.train_nr = 2;
  ci2.family = 3;
  fc2.con_info = &ci2;
  lc2._full_con = &fc2;
  route_edge._m._route_edge._conns.push_back(lc2);

  light_connection lc3(20);
  connection fc3;
  connection_info ci3;
  ci3.train_nr = 3;
  ci3.family = 2;
  fc3.con_info = &ci3;
  lc3._full_con = &fc3;
  route_edge._m._route_edge._conns.push_back(lc3);

  light_connection lc4(20);
  connection fc4;
  connection_info ci4;
  ci4.train_nr = 2;
  ci4.family = 2;
  fc4.con_info = &ci4;
  lc4._full_con = &fc4;
  route_edge._m._route_edge._conns.push_back(lc4);

  light_connection lc5(20);
  connection fc5;
  connection_info ci5;
  ci5.train_nr = 2;
  ci5.family = 2;
  fc5.con_info = &ci5;
  lc5._full_con = &fc5;
  route_edge._m._route_edge._conns.push_back(lc5);

  auto lc = find_light_connection(route_edge, 20, 2, 2, "");
  // index 3 is lc4
  ASSERT_EQ(lc.first, &route_edge._m._route_edge._conns[3]);
  ASSERT_EQ(lc.second, 3);
}

TEST_F(reliability_graph_accessor3, walking_duration) {
  auto const& tail_station =
      *get_departing_route_edge(*get_first_route_node(*schedule_, ICE_L_H))
           ->_to->_station_node;
  auto const& head_station =
      *get_first_route_node(*schedule_, S_M_W)->_station_node;
  ASSERT_EQ(walking_duration(tail_station, head_station), (duration)10);
}

TEST_F(reliability_graph_accessor3, get_interchange_time_walk) {
  auto const& feeder_arrival =
      *get_departing_route_edge(*get_first_route_node(*schedule_, ICE_L_H))
           ->_to;
  auto const& departing_train_departure =
      *get_first_route_node(*schedule_, S_M_W);
  ASSERT_EQ(get_interchange_time(feeder_arrival, departing_train_departure,
                                 *schedule_),
            (duration)10);
}

TEST_F(reliability_graph_accessor, get_interchange_time) {
  auto const& feeder_arrival =
      *get_departing_route_edge(*get_first_route_node(*schedule_, RE_MA_DA))
           ->_to;
  auto const& departing_train_departure =
      *get_departing_route_edge(*get_first_route_node(*schedule_, ICE_FR_DA_H))
           ->_to;

  ASSERT_EQ(get_interchange_time(feeder_arrival, departing_train_departure,
                                 *schedule_),
            (duration)5);
}

}  // namespace graph_accessor
}  // namespace reliability
}  // namespace motis
