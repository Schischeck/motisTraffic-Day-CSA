#include "gtest/gtest.h"

#include <iostream>

#include "motis/loader/loader.h"

#include "motis/core/common/date_time_util.h"

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
};

namespace schedule1 {
/* modules/reliability/resources/schedule/ */
constexpr auto DARMSTADT = "4219971";
constexpr auto FRANKFURT = "8351230";
/* train numbers */
constexpr unsigned ICE_FR_DA_H = 5;
constexpr unsigned RE_MA_DA = 4;
}  // namespace schedule1

class reliability_graph_accessor3 : public test_schedule_setup {
public:
  reliability_graph_accessor3()
      : test_schedule_setup("modules/reliability/resources/schedule3/",
                            "20150928") {}
};

namespace schedule3 {
constexpr unsigned ICE_L_H = 1;  // 10:00 --> 10:10
constexpr unsigned S_M_W = 2;  // 10:20 --> 10:25
}  // namespace schedule3

TEST_F(reliability_graph_accessor, get_first_route_node_by_train_nr) {
  auto node = get_first_route_node(*schedule_, schedule1::ICE_FR_DA_H);
  ASSERT_EQ(schedule_->stations_[node->station_node_->id_]->eva_nr_,
            schedule1::FRANKFURT);
  ASSERT_EQ(get_departing_route_edge(*node)
                ->m_.route_edge_.conns_[0]
                .full_con_->con_info_->train_nr_,
            schedule1::ICE_FR_DA_H);
}

TEST_F(reliability_graph_accessor, get_previous_light_connection) {
  auto const first_route_node =
      get_first_route_node(*schedule_, schedule1::ICE_FR_DA_H);
  ASSERT_EQ(schedule_->stations_[first_route_node->station_node_->id_]->eva_nr_,
            schedule1::FRANKFURT);
  auto const first_route_edge = get_departing_route_edge(*first_route_node);
  {
    auto const& first_light_conn = first_route_edge->m_.route_edge_.conns_[0];
    ASSERT_EQ(first_light_conn.d_time_,
              test_util::minutes_to_motis_time(5 * 60 + 55));

    auto second_route_node = first_route_edge->to_;
    ASSERT_EQ(
        schedule_->stations_[second_route_node->station_node_->id_]->eva_nr_,
        schedule1::DARMSTADT);
    ASSERT_EQ(get_arriving_route_edge(*second_route_node), first_route_edge);

    auto const& second_light_conn =
        get_departing_route_edge(*second_route_node)->m_.route_edge_.conns_[0];
    ASSERT_EQ(second_light_conn.d_time_,
              test_util::minutes_to_motis_time(6 * 60 + 11));
    auto const& previous_light_conn = get_previous_light_connection(
        graph_accessor::get_arriving_route_edge(*second_route_node)
            ->m_.route_edge_.conns_,
        second_light_conn.d_time_);

    ASSERT_EQ(&previous_light_conn, &first_light_conn);
  }
  {
    auto const& first_light_conn = first_route_edge->m_.route_edge_.conns_[1];
    ASSERT_EQ(first_light_conn.d_time_,
              test_util::minutes_to_motis_time(6 * 60 + 55));

    auto second_route_node = first_route_edge->to_;
    ASSERT_EQ(
        schedule_->stations_[second_route_node->station_node_->id_]->eva_nr_,
        schedule1::DARMSTADT);
    ASSERT_EQ(get_arriving_route_edge(*second_route_node), first_route_edge);

    auto const& second_light_conn =
        get_departing_route_edge(*second_route_node)->m_.route_edge_.conns_[1];
    ASSERT_EQ(second_light_conn.d_time_,
              test_util::minutes_to_motis_time(7 * 60 + 11));
    auto const& previous_light_conn = get_previous_light_connection(
        graph_accessor::get_arriving_route_edge(*second_route_node)
            ->m_.route_edge_.conns_,
        second_light_conn.d_time_);

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
  auto const first_node =
      get_first_route_node(*schedule_, schedule1::ICE_FR_DA_H);
  auto const node =
      get_departing_route_edge(*get_departing_route_edge(*first_node)->to_)
          ->to_;
  ASSERT_EQ(&get_first_route_node(*node), first_node);
}

TEST_F(reliability_graph_accessor, get_light_connection) {
  edge route_edge;
  route_edge.m_.type_ = edge::ROUTE_EDGE;

  connection fc1;
  connection_info ci1;
  ci1.train_nr_ = 1;
  ci1.family_ = 1;
  fc1.con_info_ = &ci1;
  light_connection lc1(10, 11, &fc1);
  route_edge.m_.route_edge_.conns_.push_back(lc1);

  connection fc2;
  connection_info ci2;
  ci2.train_nr_ = 2;
  ci2.family_ = 3;
  fc2.con_info_ = &ci2;
  light_connection lc2(20, 21, &fc2);
  route_edge.m_.route_edge_.conns_.push_back(lc2);

  connection fc3;
  connection_info ci3;
  ci3.train_nr_ = 3;
  ci3.family_ = 2;
  fc3.con_info_ = &ci3;
  light_connection lc3(20, 22, &fc3);
  route_edge.m_.route_edge_.conns_.push_back(lc3);

  connection fc4;
  connection_info ci4;
  ci4.train_nr_ = 2;
  ci4.family_ = 2;
  ci4.line_identifier_ = "1";
  fc4.con_info_ = &ci4;
  light_connection lc4(20, 23, &fc4);
  route_edge.m_.route_edge_.conns_.push_back(lc4);

  connection fc5;
  connection_info ci5;
  ci5.train_nr_ = 2;
  ci5.family_ = 2;
  fc5.con_info_ = &ci5;
  light_connection lc5(20, 24, &fc5);
  route_edge.m_.route_edge_.conns_.push_back(lc5);

  {
    auto lc = find_light_connection(route_edge, 20, true, 2, 2, "1");
    // index 3 is lc4
    ASSERT_EQ(lc.first, &route_edge.m_.route_edge_.conns_[3]);
    ASSERT_EQ(lc.second, 3);
  }
  {
    auto lc = find_light_connection(route_edge, 22, false, 2, 3, "");
    // index 3 is lc4
    ASSERT_EQ(lc.first, &route_edge.m_.route_edge_.conns_[2]);
    ASSERT_EQ(lc.second, 2);
  }
}

TEST_F(reliability_graph_accessor3, walking_duration) {
  auto const& tail_station =
      *get_departing_route_edge(
           *get_first_route_node(*schedule_, schedule3::ICE_L_H))
           ->to_->station_node_;
  auto const& head_station =
      *get_first_route_node(*schedule_, schedule3::S_M_W)->station_node_;
  ASSERT_EQ(walking_duration(tail_station, head_station), (duration)10);
}

TEST_F(reliability_graph_accessor3, get_interchange_time_walk) {
  auto const& feeder_arrival =
      *get_departing_route_edge(
           *get_first_route_node(*schedule_, schedule3::ICE_L_H))
           ->to_;
  auto const& departing_train_departure =
      *get_first_route_node(*schedule_, schedule3::S_M_W);
  ASSERT_EQ(get_interchange_time(feeder_arrival, departing_train_departure,
                                 *schedule_),
            (duration)10);
}

TEST_F(reliability_graph_accessor, get_interchange_time) {
  auto const& feeder_arrival =
      *get_departing_route_edge(
           *get_first_route_node(*schedule_, schedule1::RE_MA_DA))
           ->to_;
  auto const& departing_train_departure =
      *get_departing_route_edge(
           *get_first_route_node(*schedule_, schedule1::ICE_FR_DA_H))
           ->to_;

  ASSERT_EQ(get_interchange_time(feeder_arrival, departing_train_departure,
                                 *schedule_),
            (duration)5);
}

}  // namespace graph_accessor
}  // namespace reliability
}  // namespace motis
