#include "gtest/gtest.h"

#include <iostream>
#include <vector>

#include "motis/core/common/date_util.h"

#include "motis/reliability/reliability.h"
#include "motis/reliability/search/connection_graph.h"
#include "motis/reliability/search/connection_graph_search.h"
#include "motis/reliability/search/simple_connection_graph_optimizer.h"
#include "motis/reliability/tools/flatbuffers_tools.h"
#include "motis/reliability/tools/system.h"

#include "include/start_and_travel_test_distributions.h"
#include "include/test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace search {
namespace connection_graph_search {

class test_connection_graph_search : public test_schedule_setup {
public:
  test_connection_graph_search()
      : test_schedule_setup("modules/reliability/resources/schedule7_cg/",
                            to_unix_time(2015, 10, 19),
                            to_unix_time(2015, 10, 20)) {}
  schedule_station const FRANKFURT = {"Frankfurt", "1111111"};
  schedule_station const LANGEN = {"Langen", "2222222"};
  schedule_station const DARMSTADT = {"Darmstadt", "3333333"};
  short const RE_D_L = 1;  // 07:00 --> 07:10
  short const RE_L_F = 2;  // 07:15 --> 07:25
  short const S_L_F = 3;  // 07:16 --> 07:34
  short const IC_L_F = 4;  // 07:17 --> 07:40
};

TEST_F(test_connection_graph_search, reliable_routing_request) {
  system_tools::setup setup(schedule_.get());
  auto msg = flatbuffers_tools::to_reliable_routing_request(
      FRANKFURT.name, FRANKFURT.eva, DARMSTADT.name, DARMSTADT.eva,
      (motis::time)(7 * 60), (motis::time)(7 * 60 + 1),
      std::make_tuple(19, 10, 2015), RequestType_ReliableSearch);

  auto test_cb = [&](std::vector<std::shared_ptr<connection_graph> > cgs) {
    ASSERT_EQ(cgs.size(), 1);
    auto const cg = *cgs.front();

    ASSERT_EQ(cg.stops.size(), 3);
    {
      auto const& stop = cg.stops[connection_graph::stop::Index_departure_stop];
      ASSERT_EQ(stop.index, 0);
      ASSERT_EQ(stop.eva_no, "3333333");
      ASSERT_EQ(stop.name, "Darmstadt");
      ASSERT_TRUE(stop.interchange_infos.empty());
    }
    {
      auto const& stop = cg.stops[connection_graph::stop::Index_arrival_stop];
      ASSERT_EQ(stop.index, 1);
      ASSERT_EQ(stop.eva_no, "1111111");
      ASSERT_EQ(stop.name, "Frankfurt");
      ASSERT_TRUE(stop.interchange_infos.empty());
    }
    {
      auto const& stop = cg.stops[2];
      ASSERT_EQ(stop.index, 2);
      ASSERT_EQ(stop.eva_no, "2222222");
      ASSERT_EQ(stop.name, "Langen");
      ASSERT_EQ(stop.interchange_infos.size(), 3);
      {
        auto const& ic = stop.interchange_infos[0];
        ASSERT_EQ(ic.departing_journey_index, 2);
        ASSERT_EQ(ic.interchange_time, 5);
      }
      {
        auto const& ic = stop.interchange_infos[1];
        ASSERT_EQ(ic.departing_journey_index, 3);
        ASSERT_EQ(ic.interchange_time, 5);
      }
      {
        auto const& ic = stop.interchange_infos[2];
        ASSERT_EQ(ic.departing_journey_index, 4);
        ASSERT_EQ(ic.interchange_time, 5);
      }
    }

    ASSERT_EQ(cg.journeys.size(), 4);
    {
      auto const& j = cg.journeys[0];
      ASSERT_EQ(j.from_index, 0);
      ASSERT_EQ(j.to_index, 2);
      ASSERT_EQ(j.j.stops.front().eva_no, "3333333");
      ASSERT_EQ(j.j.stops.back().eva_no, "2222222");
      ASSERT_EQ(j.j.stops.front().departure.timestamp, 1445238000);  // 07:00
      ASSERT_EQ(j.j.stops.back().arrival.timestamp, 1445238600);  // 07:10
      ASSERT_EQ(j.j.transports.front().train_nr, RE_D_L);
    }
    {
      auto const& j = cg.journeys[1];
      ASSERT_EQ(j.from_index, 2);
      ASSERT_EQ(j.to_index, 1);
      ASSERT_EQ(j.j.stops.front().eva_no, "2222222");
      ASSERT_EQ(j.j.stops.back().eva_no, "1111111");
      ASSERT_EQ(j.j.stops.front().departure.timestamp, 1445238900);  // 07:15
      ASSERT_EQ(j.j.stops.back().arrival.timestamp, 1445239500);  // 07:10
      ASSERT_EQ(j.j.transports.front().train_nr, RE_L_F);
    }
    {
      auto const& j = cg.journeys[2];
      ASSERT_EQ(j.from_index, 2);
      ASSERT_EQ(j.to_index, 1);
      ASSERT_EQ(j.j.stops.front().eva_no, "2222222");
      ASSERT_EQ(j.j.stops.back().eva_no, "1111111");
      ASSERT_EQ(j.j.stops.front().departure.timestamp, 1445238960);  // 07:16
      ASSERT_EQ(j.j.stops.back().arrival.timestamp, 1445240040);  // 07:34
      ASSERT_EQ(j.j.transports.front().train_nr, S_L_F);
    }
    {
      auto const& j = cg.journeys[3];
      ASSERT_EQ(j.from_index, 2);
      ASSERT_EQ(j.to_index, 1);
      ASSERT_EQ(j.j.stops.front().eva_no, "2222222");
      ASSERT_EQ(j.j.stops.back().eva_no, "1111111");
      ASSERT_EQ(j.j.stops.front().departure.timestamp, 1445239020);  // 07:17
      ASSERT_EQ(j.j.stops.back().arrival.timestamp, 1445240400);  // 07:40
      ASSERT_EQ(j.j.transports.front().train_nr, IC_L_F);
    }
  };

  search_cgs(msg->content<ReliableRoutingRequest const*>(),
             setup.reliability_module(), *schedule_.get(), 0,
             simple_optimizer::complete, test_cb);
}

}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
