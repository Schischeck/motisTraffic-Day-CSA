#include "gtest/gtest.h"

#include <iostream>
#include <vector>

#include "motis/core/common/date_util.h"

#include "motis/reliability/reliability.h"
#include "motis/reliability/search/connection_graph.h"
#include "motis/reliability/search/connection_graph_search.h"
#include "motis/reliability/search/cg_search_context.h"
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
      DARMSTADT.name, DARMSTADT.eva, FRANKFURT.name, FRANKFURT.eva,
      (motis::time)(7 * 60), (motis::time)(7 * 60 + 1),
      std::make_tuple(19, 10, 2015), RequestType_ReliableSearch);

  bool test_cb_called = false;

  auto test_cb = [&](
      std::vector<std::shared_ptr<connection_graph> > const cgs) {
    test_cb_called = true;
    setup.ios.stop();

    ASSERT_EQ(cgs.size(), 1);
    auto const cg = *cgs.front();

    ASSERT_EQ(cg.stops_.size(), 3);
    {
      auto const& stop =
          cg.stops_[connection_graph::stop::Index_departure_stop];
      ASSERT_EQ(stop.index_, connection_graph::stop::Index_departure_stop);
      ASSERT_EQ(stop.departure_infos_.size(), 1);
      ASSERT_EQ(stop.departure_infos_.front().departing_journey_index_, 0);
      ASSERT_EQ(stop.departure_infos_.front().head_stop_index_,
                connection_graph::stop::Index_first_intermediate_stop);
    }
    {
      auto const& stop = cg.stops_[connection_graph::stop::Index_arrival_stop];
      ASSERT_EQ(stop.index_, connection_graph::stop::Index_arrival_stop);
      ASSERT_TRUE(stop.departure_infos_.empty());
    }
    {
      auto const& stop =
          cg.stops_[connection_graph::stop::Index_first_intermediate_stop];
      ASSERT_EQ(stop.index_,
                connection_graph::stop::Index_first_intermediate_stop);
      ASSERT_EQ(stop.departure_infos_.size(), 3);
      {
        auto const& ic = stop.departure_infos_[0];
        ASSERT_EQ(ic.departing_journey_index_, 1);
        ASSERT_EQ(ic.head_stop_index_,
                  connection_graph::stop::Index_arrival_stop);
        // ASSERT_EQ(ic.interchange_time, 5);
      }
      {
        auto const& ic = stop.departure_infos_[1];
        ASSERT_EQ(ic.departing_journey_index_, 2);
        ASSERT_EQ(ic.head_stop_index_,
                  connection_graph::stop::Index_arrival_stop);
        // ASSERT_EQ(ic.interchange_time, 5);
      }
      {
        auto const& ic = stop.departure_infos_[2];
        ASSERT_EQ(ic.departing_journey_index_, 3);
        ASSERT_EQ(ic.head_stop_index_,
                  connection_graph::stop::Index_arrival_stop);
        // ASSERT_EQ(ic.interchange_time, 5);
      }
    }

    ASSERT_EQ(cg.journeys_.size(), 4);
    {
      auto const& j = cg.journeys_[0];
      ASSERT_EQ(j.j_.stops.front().eva_no, "3333333");
      ASSERT_EQ(j.j_.stops.back().eva_no, "2222222");
      ASSERT_EQ(j.j_.stops.front().departure.timestamp, 1445238000);  // 07:00
      ASSERT_EQ(j.j_.stops.back().arrival.timestamp, 1445238600);  // 07:10
      ASSERT_EQ(j.j_.transports.front().train_nr, RE_D_L);
    }
    {
      auto const& j = cg.journeys_[1];
      ASSERT_EQ(j.j_.stops.front().eva_no, "2222222");
      ASSERT_EQ(j.j_.stops.back().eva_no, "1111111");
      ASSERT_EQ(j.j_.stops.front().departure.timestamp, 1445238900);  // 07:15
      ASSERT_EQ(j.j_.stops.back().arrival.timestamp, 1445239500);  // 07:10
      ASSERT_EQ(j.j_.transports.front().train_nr, RE_L_F);
    }
    {
      auto const& j = cg.journeys_[2];
      ASSERT_EQ(j.j_.stops.front().eva_no, "2222222");
      ASSERT_EQ(j.j_.stops.back().eva_no, "1111111");
      ASSERT_EQ(j.j_.stops.front().departure.timestamp, 1445238960);  // 07:16
      ASSERT_EQ(j.j_.stops.back().arrival.timestamp, 1445240040);  // 07:34
      ASSERT_EQ(j.j_.transports.front().train_nr, S_L_F);
    }
    {
      auto const& j = cg.journeys_[3];
      ASSERT_EQ(j.j_.stops.front().eva_no, "2222222");
      ASSERT_EQ(j.j_.stops.back().eva_no, "1111111");
      ASSERT_EQ(j.j_.stops.front().departure.timestamp, 1445239020);  // 07:17
      ASSERT_EQ(j.j_.stops.back().arrival.timestamp, 1445240400);  // 07:40
      ASSERT_EQ(j.j_.transports.front().train_nr, IC_L_F);
    }

  };

  boost::asio::io_service::work ios_work(setup.ios);

  simple_optimizer optimizer(3, 1, 15);
  search_cgs(msg->content<ReliableRoutingRequest const*>(),
             setup.reliability_module(), 0, optimizer, test_cb);

  setup.ios.run();

  ASSERT_TRUE(test_cb_called);
}

/* search for alternatives at stops not necessary
 * (base connection is optimal) */
TEST_F(test_connection_graph_search, reliable_routing_request2) {
  system_tools::setup setup(schedule_.get());
  auto msg = flatbuffers_tools::to_reliable_routing_request(
      DARMSTADT.name, DARMSTADT.eva, FRANKFURT.name, FRANKFURT.eva,
      (motis::time)(7 * 60), (motis::time)(7 * 60 + 1),
      std::make_tuple(19, 10, 2015), RequestType_ReliableSearch);

  bool test_cb_called = false;

  auto test_cb = [&](
      std::vector<std::shared_ptr<connection_graph> > const cgs) {
    test_cb_called = true;
    setup.ios.stop();

    ASSERT_EQ(cgs.size(), 1);
    auto const cg = *cgs.front();

    ASSERT_EQ(cg.stops_.size(), 3);
    {
      auto const& stop =
          cg.stops_[connection_graph::stop::Index_departure_stop];
      ASSERT_EQ(stop.index_, connection_graph::stop::Index_departure_stop);
      ASSERT_EQ(stop.departure_infos_.size(), 1);
      ASSERT_EQ(stop.departure_infos_.front().departing_journey_index_, 0);
      ASSERT_EQ(stop.departure_infos_.front().head_stop_index_,
                connection_graph::stop::Index_first_intermediate_stop);
    }
    {
      auto const& stop = cg.stops_[connection_graph::stop::Index_arrival_stop];
      ASSERT_EQ(stop.index_, connection_graph::stop::Index_arrival_stop);
      ASSERT_TRUE(stop.departure_infos_.empty());
    }
    {
      auto const& stop =
          cg.stops_[connection_graph::stop::Index_first_intermediate_stop];
      ASSERT_EQ(stop.index_,
                connection_graph::stop::Index_first_intermediate_stop);
      ASSERT_EQ(stop.departure_infos_.size(), 1);
      {
        auto const& ic = stop.departure_infos_[0];
        ASSERT_EQ(ic.departing_journey_index_, 1);
        ASSERT_EQ(ic.head_stop_index_,
                  connection_graph::stop::Index_arrival_stop);
        // ASSERT_EQ(ic.interchange_time, 5);
      }
    }

    ASSERT_EQ(cg.journeys_.size(), 2);
    {
      auto const& j = cg.journeys_[0];
      ASSERT_EQ(j.j_.stops.front().eva_no, "3333333");
      ASSERT_EQ(j.j_.stops.back().eva_no, "2222222");
      ASSERT_EQ(j.j_.stops.front().departure.timestamp, 1445238000);  // 07:00
      ASSERT_EQ(j.j_.stops.back().arrival.timestamp, 1445238600);  // 07:10
      ASSERT_EQ(j.j_.transports.front().train_nr, RE_D_L);
    }
    {
      auto const& j = cg.journeys_[1];
      ASSERT_EQ(j.j_.stops.front().eva_no, "2222222");
      ASSERT_EQ(j.j_.stops.back().eva_no, "1111111");
      ASSERT_EQ(j.j_.stops.front().departure.timestamp, 1445238900);  // 07:15
      ASSERT_EQ(j.j_.stops.back().arrival.timestamp, 1445239500);  // 07:10
      ASSERT_EQ(j.j_.transports.front().train_nr, RE_L_F);
    }
  };

  boost::asio::io_service::work ios_work(setup.ios);

  simple_optimizer optimizer(1, 1, 15);
  search_cgs(msg->content<ReliableRoutingRequest const*>(),
             setup.reliability_module(), 0, optimizer, test_cb);

  setup.ios.run();

  ASSERT_TRUE(test_cb_called);
}

TEST_F(test_connection_graph_search, cache_journey) {
  simple_optimizer optimizer(1, 1, 1);
  motis::reliability::reliability rel;
  callback cb;
  detail::context c(rel, 0, cb, optimizer);
  using key = detail::context::journey_cache_key;
  {
    journey j;
    j.duration = 10;
    c.journey_cache[key("A", 0, 3)] = j;
  }
  {
    journey j;
    j.duration = 20;
    c.journey_cache[key("B", 2, 5)] = j;
    ASSERT_EQ(c.journey_cache.end(), c.journey_cache.find(key("B", 2, 6)));
  }
  {
    journey j;
    j.duration = 30;
    c.journey_cache[key("B", 2, 6)] = j;
  }
  {
    journey j;
    j.duration = 40;
    c.journey_cache[key("B", 3, 5)] = j;
  }
  {
    journey j;
    j.duration = 50;
    c.journey_cache[key("B", 1, 4)] = j;
  }

  ASSERT_EQ(10, c.journey_cache.at(key("A", 0, 3)).duration);
  ASSERT_EQ(20, c.journey_cache.at(key("B", 2, 5)).duration);
  ASSERT_EQ(30, c.journey_cache.at(key("B", 2, 6)).duration);
  ASSERT_EQ(40, c.journey_cache.at(key("B", 3, 5)).duration);
  ASSERT_EQ(50, c.journey_cache.at(key("B", 1, 4)).duration);

  ASSERT_EQ(c.journey_cache.end(), c.journey_cache.find(key("A", 0, 0)));
  ASSERT_EQ(c.journey_cache.end(), c.journey_cache.find(key("A", 3, 3)));
  ASSERT_EQ(c.journey_cache.end(), c.journey_cache.find(key("A", 0, 2)));
  ASSERT_EQ(c.journey_cache.end(), c.journey_cache.find(key("A", 1, 3)));
  ASSERT_EQ(c.journey_cache.end(), c.journey_cache.find(key("B", 0, 0)));
  ASSERT_EQ(c.journey_cache.end(), c.journey_cache.find(key("B", 2, 3)));
  ASSERT_EQ(c.journey_cache.end(), c.journey_cache.find(key("B", 6, 6)));
  ASSERT_EQ(c.journey_cache.end(), c.journey_cache.find(key("B", 3, 4)));
  ASSERT_EQ(c.journey_cache.end(), c.journey_cache.find(key("C", 0, 3)));
}

}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
