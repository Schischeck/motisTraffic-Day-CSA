#include "gtest/gtest.h"

#include <iostream>
#include <vector>

#include "motis/core/common/date_time_util.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/reliability.h"
#include "motis/reliability/search/cg_optimizer.h"
#include "motis/reliability/search/cg_search_context.h"
#include "motis/reliability/search/connection_graph.h"
#include "motis/reliability/search/connection_graph_search.h"
#include "motis/reliability/tools/flatbuffers/request_builder.h"

#include "../include/message_builder.h"
#include "../include/schedules/schedule_realtime_cg.h"
#include "../include/start_and_travel_test_distributions.h"
#include "../include/test_schedule_setup.h"
#include "../include/test_util.h"

namespace motis {
namespace reliability {
namespace search {
namespace connection_graph_search {

using namespace module;

class reliability_realtime_cg : public test_motis_setup {
public:
  reliability_realtime_cg()
      : test_motis_setup(schedule_realtime_cg::PATH, schedule_realtime_cg::DATE,
                         true) {}

  void test_scheduled_cg(msg_ptr msg) {
    auto res = motis_content(ReliableRoutingResponse, msg);
    ASSERT_EQ(1, res->connection_graphs()->size());
    auto cg = *res->connection_graphs()->begin();
    ASSERT_EQ(3, cg->journeys()->size());
    {
      auto const& j = (*cg->journeys())[1];
      ASSERT_EQ(1445231700 /* 10/19/2015, 7:15:00 AM GMT+2:00 DST */,
                (*j->stops())[0]->departure()->time());
      ASSERT_EQ(schedule_realtime_cg::RE_L_F,
                static_cast<Transport const*>(j->transports()->begin()->move())
                    ->train_nr());
    }
  }

  void test_realtime_cg(msg_ptr msg) {
    auto res = motis_content(ReliableRoutingResponse, msg);
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
        ASSERT_EQ(1445230800 /* 2015-10-19 07:00:00 GMT+2:00 */,
                  dep_dist->begin_time());
        ASSERT_EQ(2, dep_dist->distribution()->size());
        ASSERT_FLOAT_EQ(0.8, (*dep_dist->distribution())[0]);
        ASSERT_FLOAT_EQ(0.2, (*dep_dist->distribution())[1]);
      }
      {
        auto const* arr_dist = alternative->rating()->arrival_distribution();
        ASSERT_EQ(1445231460 /* 2015-10-19 07:11:00 GMT+2:00 */,
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
      ASSERT_EQ(1445231400 /* 10/19/2015, 7:10:00 AM GMT+2:00 DST */,
                (*j->stops())[j->stops()->size() - 1]
                    ->arrival()
                    ->schedule_time());  // 07:10 is-delayed
      ASSERT_EQ(1445231460 /* 10/19/2015, 7:11:00 AM GMT+2:00 DST */,
                (*j->stops())[j->stops()->size() - 1]
                    ->arrival()
                    ->time());  // 07:11 is-delayed
      ASSERT_EQ(schedule_realtime_cg::RE_D_L,
                static_cast<Transport const*>(j->transports()->begin()->move())
                    ->train_nr());
    }
    {
      auto const& j = (*cg->journeys())[1];
      ASSERT_EQ(1445231760 /* 10/19/2015, 7:16:00 AM GMT+2:00 DST */,
                (*j->stops())[0]->departure()->time());  // 07:16
      ASSERT_EQ(schedule_realtime_cg::RE_L_F,
                static_cast<Transport const*>(j->transports()->begin()->move())
                    ->train_nr());
    }
    {
      auto const& j = (*cg->journeys())[2];
      ASSERT_EQ(1445231820 /* 10/19/2015, 7:17:00 AM GMT+2:00 DST*/,
                (*j->stops())[0]->departure()->time());  // 07:17
      ASSERT_EQ(schedule_realtime_cg::IC_L_F,
                static_cast<Transport const*>(j->transports()->begin()->move())
                    ->train_nr());
    }
  }
};

TEST_F(reliability_realtime_cg, reliable_routing_request) {
  test_scheduled_cg(call(
      flatbuffers::request_builder()
          .add_pretrip_start(schedule_realtime_cg::DARMSTADT.name_,
                             schedule_realtime_cg::DARMSTADT.eva_,
                             test_util::hhmm_to_unixtime(get_schedule(), 700),
                             test_util::hhmm_to_unixtime(get_schedule(), 701))
          .add_destination(schedule_realtime_cg::FRANKFURT.name_,
                           schedule_realtime_cg::FRANKFURT.eva_)
          .build_reliable_search_request(1, false, false, false)));

  publish(realtime::get_delay_message(
      schedule_realtime_cg::LANGEN.eva_, schedule_realtime_cg::RE_D_L, "",
      ris::EventType_Arrival, 1445231400 /* 2015-10-19 07:10:00 GMT+2:00 */,
      1445231460 /* 2015-10-19 07:11:00 GMT+2:00 */,
      schedule_realtime_cg::DARMSTADT.eva_, schedule_realtime_cg::RE_D_L,
      1445230800 /* 2015-10-19 07:00:00 GMT+2:00 */, ris::DelayType_Is));
  publish(make_no_msg("/ris/system_time_changed"));

  ASSERT_EQ(test_util::minutes_to_motis_time(7 * 60 + 11),
            graph_accessor::get_departing_route_edge(
                *graph_accessor::get_first_route_node(
                    get_schedule(), schedule_realtime_cg::RE_D_L))
                ->m_.route_edge_.conns_[0]
                .a_time_);

  publish(realtime::get_delay_message(
      schedule_realtime_cg::LANGEN.eva_, schedule_realtime_cg::RE_L_F, "",
      ris::EventType_Departure, 1445231700 /* 2015-10-19 07:15:00 GMT+2:00 */,
      1445231760 /* 2015-10-19 07:16:00 GMT+2:00 */,
      schedule_realtime_cg::LANGEN.eva_, schedule_realtime_cg::RE_L_F,
      1445231700 /* 2015-10-19 07:15:00 GMT+2:00 */, ris::DelayType_Forecast));
  publish(make_no_msg("/ris/system_time_changed"));

  test_realtime_cg(call(
      flatbuffers::request_builder()
          .add_pretrip_start(schedule_realtime_cg::DARMSTADT.name_,
                             schedule_realtime_cg::DARMSTADT.eva_,
                             test_util::hhmm_to_unixtime(get_schedule(), 700),
                             test_util::hhmm_to_unixtime(get_schedule(), 701))
          .add_destination(schedule_realtime_cg::FRANKFURT.name_,
                           schedule_realtime_cg::FRANKFURT.eva_)
          .build_reliable_search_request(1, false, false, false)));
}

TEST_F(reliability_realtime_cg, cg_arrival_distribution_is) {
  publish(realtime::get_delay_message(
      schedule_realtime_cg::LANGEN.eva_, schedule_realtime_cg::RE_D_L, "",
      ris::EventType_Arrival, 1445231400 /* 2015-10-19 07:10:00 GMT+2:00 */,
      1445231460 /* 2015-10-19 07:11:00 GMT+2:00 */,
      schedule_realtime_cg::DARMSTADT.eva_, schedule_realtime_cg::RE_D_L,
      1445230800 /* 2015-10-19 07:00:00 GMT+2:00 */, ris::DelayType_Is));
  publish(make_no_msg("/ris/system_time_changed"));

  auto req_msg =
      flatbuffers::request_builder()
          .add_pretrip_start(schedule_realtime_cg::DARMSTADT.name_,
                             schedule_realtime_cg::DARMSTADT.eva_,
                             test_util::hhmm_to_unixtime(get_schedule(), 700),
                             test_util::hhmm_to_unixtime(get_schedule(), 701))
          .add_destination(schedule_realtime_cg::LANGEN.name_,
                           schedule_realtime_cg::LANGEN.eva_)
          .build_reliable_search_request(1, false, false, false);
  auto msg = call(req_msg);

  auto res = motis_content(ReliableRoutingResponse, msg);
  ASSERT_EQ(1, res->connection_graphs()->size());
  auto cg = *res->connection_graphs()->begin();

  ASSERT_EQ(1445231460 /* 2015-10-19 07:11:00 GMT+2:00 */,
            cg->arrival_distribution()->begin_time());
  ASSERT_EQ(1, cg->arrival_distribution()->distribution()->size());
  ASSERT_FLOAT_EQ(1.0, *cg->arrival_distribution()->distribution()->begin());
}

TEST_F(reliability_realtime_cg, cg_arrival_distribution_forecast) {
  publish(realtime::get_delay_message(
      schedule_realtime_cg::LANGEN.eva_, schedule_realtime_cg::RE_D_L, "",
      ris::EventType_Arrival, 1445231400 /* 2015-10-19 07:10:00 GMT+2:00 */,
      1445231460 /* 2015-10-19 07:11:00 GMT+2:00 */,
      schedule_realtime_cg::DARMSTADT.eva_, schedule_realtime_cg::RE_D_L,
      1445230800 /* 2015-10-19 07:00:00 GMT+2:00 */, ris::DelayType_Forecast));
  publish(make_no_msg("/ris/system_time_changed"));

  auto req_msg =
      flatbuffers::request_builder()
          .add_pretrip_start(schedule_realtime_cg::DARMSTADT.name_,
                             schedule_realtime_cg::DARMSTADT.eva_,
                             test_util::hhmm_to_unixtime(get_schedule(), 700),
                             test_util::hhmm_to_unixtime(get_schedule(), 701))
          .add_destination(schedule_realtime_cg::LANGEN.name_,
                           schedule_realtime_cg::LANGEN.eva_)
          .build_reliable_search_request(1, false, false, false);
  auto msg = call(req_msg);

  auto res = motis_content(ReliableRoutingResponse, msg);
  ASSERT_EQ(1, res->connection_graphs()->size());
  auto cg = *res->connection_graphs()->begin();

  ASSERT_EQ(1445231340 /* 2015-10-19 07:09:00 GMT+2:00 */,
            cg->arrival_distribution()->begin_time());
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
