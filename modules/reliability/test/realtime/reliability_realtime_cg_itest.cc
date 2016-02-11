#include "gtest/gtest.h"

#include <iostream>
#include <vector>

#include "motis/core/common/date_util.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/reliability.h"
#include "motis/reliability/search/connection_graph.h"
#include "motis/reliability/search/connection_graph_search.h"
#include "motis/reliability/search/cg_search_context.h"
#include "motis/reliability/search/cg_optimizer.h"
#include "motis/reliability/tools/flatbuffers/request_builder.h"

#include "../include/message_builder.h"
#include "../include/start_and_travel_test_distributions.h"
#include "../include/test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace search {
namespace connection_graph_search {

class reliability_realtime_cg : public test_motis_setup {
public:
  reliability_realtime_cg()
      : test_motis_setup("modules/reliability/resources/schedule_realtime_cg/",
                         "20151019", true) {}

  schedule_station const FRANKFURT = {"Frankfurt", "1111111"};
  schedule_station const LANGEN = {"Langen", "2222222"};
  schedule_station const DARMSTADT = {"Darmstadt", "3333333"};
  short const RE_D_L = 1;  // 07:00 --> 07:10 (+1 is-delay)
  short const RE_L_F = 2;  // 07:15 (+1 forecast) --> 07:25
  short const IC_L_F = 4;  // 07:17 --> 07:40

  void test_scheduled_cg(module::msg_ptr msg) {
    ASSERT_NE(nullptr, msg);
    ASSERT_EQ(MsgContent_ReliableRoutingResponse, msg->content_type());
    auto res = msg->content<ReliableRoutingResponse const*>();
    ASSERT_EQ(1, res->connection_graphs()->size());
    auto cg = *res->connection_graphs()->begin();
    ASSERT_EQ(3, cg->journeys()->size());
    {
      auto const& j = (*cg->journeys())[1];
      ASSERT_EQ(1445238900, (*j->stops())[0]->departure()->time());  // 07:15
      ASSERT_EQ(RE_L_F,
                ((routing::Transport const*)j->transports()->begin()->move())
                    ->train_nr());
    }
  }

  void test_realtime_cg(module::msg_ptr msg) {
    ASSERT_NE(nullptr, msg);
    ASSERT_EQ(MsgContent_ReliableRoutingResponse, msg->content_type());
    auto res = msg->content<ReliableRoutingResponse const*>();
    ASSERT_EQ(1, res->connection_graphs()->size());
    auto cg = *res->connection_graphs()->begin();
    ASSERT_EQ(3, cg->stops()->size());

    {
      auto const& stop = (*cg->stops())[0];
      ASSERT_EQ(stop->alternatives()->size(), 1);
      auto const& alternative = stop->alternatives()->begin();
      ASSERT_EQ(0, alternative->journey());
      {
        auto const* dep_dist = alternative->rating()->departure_distribution();
        ASSERT_EQ(1445238000 /* 2015-10-19 07:00:00 GMT */,
                  dep_dist->begin_time());
        ASSERT_EQ(2, dep_dist->distribution()->size());
        ASSERT_FLOAT_EQ(0.8, (*dep_dist->distribution())[0]);
        ASSERT_FLOAT_EQ(0.2, (*dep_dist->distribution())[1]);
      }
      {
        auto const* arr_dist = alternative->rating()->arrival_distribution();
        ASSERT_EQ(1445238660 /* 2015-10-19 07:11:00 GMT */,
                  arr_dist->begin_time());
        ASSERT_EQ(1, arr_dist->distribution()->size());
        ASSERT_FLOAT_EQ(1.0, (*arr_dist->distribution()->begin()));
      }
    }
    {
      auto const& stop = (*cg->stops())[2];
      ASSERT_EQ(stop->alternatives()->size(), 2);
      ASSERT_EQ(1, stop->alternatives()->begin()->journey());
    }

    ASSERT_EQ(3, cg->journeys()->size());
    {
      auto const& j = cg->journeys()->begin();
      ASSERT_EQ(1445238600, (*j->stops())[j->stops()->size() - 1]
                                ->arrival()
                                ->schedule_time());  // 07:10 is-delayed
      ASSERT_EQ(1445238660, (*j->stops())[j->stops()->size() - 1]
                                ->arrival()
                                ->time());  // 07:11 is-delayed
      ASSERT_EQ(RE_D_L,
                ((routing::Transport const*)j->transports()->begin()->move())
                    ->train_nr());
    }
    {
      auto const& j = (*cg->journeys())[1];
      ASSERT_EQ(1445238960, (*j->stops())[0]->departure()->time());  // 07:16
      ASSERT_EQ(RE_L_F,
                ((routing::Transport const*)j->transports()->begin()->move())
                    ->train_nr());
    }
    {
      auto const& j = (*cg->journeys())[2];
      ASSERT_EQ(1445239020, (*j->stops())[0]->departure()->time());  // 07:17
      ASSERT_EQ(IC_L_F,
                ((routing::Transport const*)j->transports()->begin()->move())
                    ->train_nr());
    }
  }
};

TEST_F(reliability_realtime_cg, reliable_routing_request) {
  test_scheduled_cg(test::send(
      motis_instance_,
      flatbuffers::request_builder::request_builder()
          .add_station(DARMSTADT.name, DARMSTADT.eva)
          .add_station(FRANKFURT.name, FRANKFURT.eva)
          .set_interval(std::make_tuple(19, 10, 2015), (motis::time)(7 * 60),
                        (motis::time)(7 * 60 + 1))
          .build_reliable_search_request(1)));

  test::send(motis_instance_,
             realtime::get_delay_message(
                 LANGEN.eva, RE_D_L, 1445238600 /* 2015-10-19 07:10:00 GMT */,
                 1445238660 /* 2015-10-19 07:11:00 GMT */,
                 ris::EventType_Arrival, ris::DelayType_Is));

  ASSERT_EQ(to_motis_time(7 * 60 + 11),
            graph_accessor::get_departing_route_edge(
                *graph_accessor::get_first_route_node(get_schedule(), RE_D_L))
                ->_m._route_edge._conns[0]
                .a_time);

  test::send(motis_instance_,
             realtime::get_delay_message(
                 LANGEN.eva, RE_L_F, 1445238900 /* 2015-10-19 07:15:00 GMT */,
                 1445238960 /* 2015-10-19 07:16:00 GMT */,
                 ris::EventType_Departure, ris::DelayType_Forecast));

  test_realtime_cg(test::send(
      motis_instance_,
      flatbuffers::request_builder::request_builder()
          .add_station(DARMSTADT.name, DARMSTADT.eva)
          .add_station(FRANKFURT.name, FRANKFURT.eva)
          .set_interval(std::make_tuple(19, 10, 2015), (motis::time)(7 * 60),
                        (motis::time)(7 * 60 + 1))
          .build_reliable_search_request(1)));
}

TEST_F(reliability_realtime_cg, cg_arrival_distribution_is) {
  test::send(motis_instance_,
             realtime::get_delay_message(
                 LANGEN.eva, RE_D_L, 1445238600 /* 2015-10-19 07:10:00 GMT */,
                 1445238660 /* 2015-10-19 07:11:00 GMT */,
                 ris::EventType_Arrival, ris::DelayType_Is));
  auto req_msg =
      flatbuffers::request_builder::request_builder()
          .add_station(DARMSTADT.name, DARMSTADT.eva)
          .add_station(LANGEN.name, LANGEN.eva)
          .set_interval(std::make_tuple(19, 10, 2015), (motis::time)(7 * 60),
                        (motis::time)(7 * 60 + 1))
          .build_reliable_search_request(1);
  auto msg = test::send(motis_instance_, req_msg);
  ASSERT_NE(nullptr, msg);

  auto res = msg->content<ReliableRoutingResponse const*>();
  ASSERT_EQ(1, res->connection_graphs()->size());
  auto cg = *res->connection_graphs()->begin();

  ASSERT_EQ(1445238660, cg->arrival_distribution()->begin_time());
  ASSERT_EQ(1, cg->arrival_distribution()->distribution()->size());
  ASSERT_FLOAT_EQ(1.0, *cg->arrival_distribution()->distribution()->begin());
}

TEST_F(reliability_realtime_cg, cg_arrival_distribution_forecast) {
  test::send(motis_instance_,
             realtime::get_delay_message(
                 LANGEN.eva, RE_D_L, 1445238600 /* 2015-10-19 07:10:00 GMT */,
                 1445238660 /* 2015-10-19 07:11:00 GMT */,
                 ris::EventType_Arrival, ris::DelayType_Forecast));

  auto req_msg =
      flatbuffers::request_builder::request_builder()
          .add_station(DARMSTADT.name, DARMSTADT.eva)
          .add_station(LANGEN.name, LANGEN.eva)
          .set_interval(std::make_tuple(19, 10, 2015), (motis::time)(7 * 60),
                        (motis::time)(7 * 60 + 1))
          .build_reliable_search_request(1);
  auto msg = test::send(motis_instance_, req_msg);
  ASSERT_NE(nullptr, msg);
  auto res = msg->content<ReliableRoutingResponse const*>();
  ASSERT_EQ(1, res->connection_graphs()->size());
  auto cg = *res->connection_graphs()->begin();

  ASSERT_EQ(1445238540, cg->arrival_distribution()->begin_time());
  ASSERT_EQ(4, cg->arrival_distribution()->distribution()->size());
  ASSERT_FLOAT_EQ(0.08, (*cg->arrival_distribution()->distribution())[0]);
  ASSERT_FLOAT_EQ(0.66, (*cg->arrival_distribution()->distribution())[1]);
  ASSERT_FLOAT_EQ(0.24, (*cg->arrival_distribution()->distribution())[2]);
  ASSERT_FLOAT_EQ(0.02, (*cg->arrival_distribution()->distribution())[3]);
}

}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
