#include "gtest/gtest.h"

#include <iostream>
#include <vector>

#include "motis/core/common/date_util.h"

#include "motis/reliability/reliability.h"
#include "motis/reliability/search/connection_graph.h"
#include "motis/reliability/search/connection_graph_search.h"
#include "motis/reliability/search/cg_search_context.h"
#include "motis/reliability/search/cg_optimizer.h"
#include "motis/reliability/tools/flatbuffers/request_builder.h"

#include "../include/start_and_travel_test_distributions.h"
#include "../include/test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace search {
namespace connection_graph_search {

class reliability_connection_graph_search : public test_motis_setup {
public:
  reliability_connection_graph_search()
      : test_motis_setup("modules/reliability/resources/schedule7_cg/",
                         "20151019") {}

  schedule_station const FRANKFURT = {"Frankfurt", "1111111"};
  schedule_station const LANGEN = {"Langen", "2222222"};
  schedule_station const DARMSTADT = {"Darmstadt", "3333333"};
  short const RE_D_L = 1;  // 07:00 --> 07:10
  short const RE_L_F = 2;  // 07:15 --> 07:25
  short const S_L_F = 3;  // 07:16 --> 07:34
  short const IC_L_F = 4;  // 07:17 --> 07:40

  void test_cg(std::vector<std::shared_ptr<connection_graph> > const cgs,
               std::shared_ptr<bool> test_cb_called) {
    *test_cb_called = true;

    ASSERT_EQ(1, cgs.size());
    auto const cg = *cgs.front();

    ASSERT_EQ(3, cg.stops_.size());
    {
      auto const& stop =
          cg.stops_[connection_graph::stop::Index_departure_stop];
      ASSERT_EQ(stop.index_, connection_graph::stop::Index_departure_stop);
      ASSERT_EQ(stop.alternative_infos_.size(), 1);
      ASSERT_EQ(stop.alternative_infos_.front().journey_index_, 0);
      ASSERT_EQ(stop.alternative_infos_.front().next_stop_index_,
                connection_graph::stop::Index_first_intermediate_stop);
    }
    {
      auto const& stop = cg.stops_[connection_graph::stop::Index_arrival_stop];
      ASSERT_EQ(stop.index_, connection_graph::stop::Index_arrival_stop);
      ASSERT_TRUE(stop.alternative_infos_.empty());
    }
    {
      auto const& stop =
          cg.stops_[connection_graph::stop::Index_first_intermediate_stop];
      ASSERT_EQ(stop.index_,
                connection_graph::stop::Index_first_intermediate_stop);
      ASSERT_EQ(stop.alternative_infos_.size(), 3);
      {
        auto const& ic = stop.alternative_infos_[0];
        ASSERT_EQ(ic.journey_index_, 1);
        ASSERT_EQ(ic.next_stop_index_,
                  connection_graph::stop::Index_arrival_stop);
        // ASSERT_EQ(ic.interchange_time, 5);
      }
      {
        auto const& ic = stop.alternative_infos_[1];
        ASSERT_EQ(ic.journey_index_, 2);
        ASSERT_EQ(ic.next_stop_index_,
                  connection_graph::stop::Index_arrival_stop);
        // ASSERT_EQ(ic.interchange_time, 5);
      }
      {
        auto const& ic = stop.alternative_infos_[2];
        ASSERT_EQ(ic.journey_index_, 3);
        ASSERT_EQ(ic.next_stop_index_,
                  connection_graph::stop::Index_arrival_stop);
        // ASSERT_EQ(ic.interchange_time, 5);
      }
    }

    ASSERT_EQ(cg.journeys_.size(), 4);
    {
      auto const& j = cg.journeys_[0];
      ASSERT_EQ(j.stops.front().eva_no, "3333333");
      ASSERT_EQ(j.stops.back().eva_no, "2222222");
      ASSERT_EQ(j.stops.front().departure.timestamp, 1445238000);  // 07:00
      ASSERT_EQ(j.stops.back().arrival.timestamp, 1445238600);  // 07:10
      ASSERT_EQ(j.transports.front().train_nr, RE_D_L);
    }
    {
      auto const& j = cg.journeys_[1];
      ASSERT_EQ(j.stops.front().eva_no, "2222222");
      ASSERT_EQ(j.stops.back().eva_no, "1111111");
      ASSERT_EQ(j.stops.front().departure.timestamp, 1445238900);  // 07:15
      ASSERT_EQ(j.stops.back().arrival.timestamp, 1445239500);  // 07:25
      ASSERT_EQ(j.transports.front().train_nr, RE_L_F);
    }
    {
      auto const& j = cg.journeys_[2];
      ASSERT_EQ(j.stops.front().eva_no, "2222222");
      ASSERT_EQ(j.stops.back().eva_no, "1111111");
      ASSERT_EQ(j.stops.front().departure.timestamp, 1445238960);  // 07:16
      ASSERT_EQ(j.stops.back().arrival.timestamp, 1445240040);  // 07:34
      ASSERT_EQ(j.transports.front().train_nr, S_L_F);
    }
    {
      auto const& j = cg.journeys_[3];
      ASSERT_EQ(j.stops.front().eva_no, "2222222");
      ASSERT_EQ(j.stops.back().eva_no, "1111111");
      ASSERT_EQ(j.stops.front().departure.timestamp, 1445239020);  // 07:17
      ASSERT_EQ(j.stops.back().arrival.timestamp, 1445240400);  // 07:40
      ASSERT_EQ(j.transports.front().train_nr, IC_L_F);
    }
  }
};

TEST_F(reliability_connection_graph_search, reliable_routing_request) {
  auto msg = flatbuffers::request_builder::request_builder()
                 .add_station(DARMSTADT.name, DARMSTADT.eva)
                 .add_station(FRANKFURT.name, FRANKFURT.eva)
                 .set_interval(std::make_tuple(19, 10, 2015),
                               (motis::time)(7 * 60), (motis::time)(7 * 60 + 1))
                 .build_connection_tree_request(3, 1);
  auto test_cb_called = std::make_shared<bool>(false);
  search_cgs(msg->content<ReliableRoutingRequest const*>(),
             get_reliability_module(), 0,
             std::make_shared<simple_optimizer>(3, 1),
             std::bind(&reliability_connection_graph_search::test_cg, this,
                       std::placeholders::_1, test_cb_called));
  motis_instance_->run();
  ASSERT_TRUE(*test_cb_called);
}

/* optimize connection graph alternatives depending on distributions! */
TEST_F(reliability_connection_graph_search,
       reliable_routing_request_optimization) {
  auto msg = flatbuffers::request_builder::request_builder()
                 .add_station(DARMSTADT.name, DARMSTADT.eva)
                 .add_station(FRANKFURT.name, FRANKFURT.eva)
                 .set_interval(std::make_tuple(19, 10, 2015),
                               (motis::time)(7 * 60), (motis::time)(7 * 60 + 1))
                 .build_reliable_search_request(1);
  auto test_cb_called = std::make_shared<bool>(false);
  search_cgs(msg->content<ReliableRoutingRequest const*>(),
             get_reliability_module(), 0,
             std::make_shared<reliable_cg_optimizer>(1),
             std::bind(&reliability_connection_graph_search::test_cg, this,
                       std::placeholders::_1, test_cb_called));
  motis_instance_->run();
  ASSERT_TRUE(*test_cb_called);
}

/* search for alternatives at stops not necessary
 * (base connection is optimal) */
TEST_F(reliability_connection_graph_search, reliable_routing_request2) {
  auto msg = flatbuffers::request_builder::request_builder()
                 .add_station(DARMSTADT.name, DARMSTADT.eva)
                 .add_station(FRANKFURT.name, FRANKFURT.eva)
                 .set_interval(std::make_tuple(19, 10, 2015),
                               (motis::time)(7 * 60), (motis::time)(7 * 60 + 1))
                 .build_connection_tree_request(1, 1);

  bool test_cb_called = false;

  auto test_cb = [&](
      std::vector<std::shared_ptr<connection_graph> > const cgs) {
    test_cb_called = true;

    ASSERT_EQ(cgs.size(), 1);
    auto const cg = *cgs.front();

    ASSERT_EQ(cg.stops_.size(), 3);
    {
      auto const& stop =
          cg.stops_[connection_graph::stop::Index_departure_stop];
      ASSERT_EQ(stop.index_, connection_graph::stop::Index_departure_stop);
      ASSERT_EQ(stop.alternative_infos_.size(), 1);
      ASSERT_EQ(stop.alternative_infos_.front().journey_index_, 0);
      ASSERT_EQ(stop.alternative_infos_.front().next_stop_index_,
                connection_graph::stop::Index_first_intermediate_stop);
    }
    {
      auto const& stop = cg.stops_[connection_graph::stop::Index_arrival_stop];
      ASSERT_EQ(stop.index_, connection_graph::stop::Index_arrival_stop);
      ASSERT_TRUE(stop.alternative_infos_.empty());
    }
    {
      auto const& stop =
          cg.stops_[connection_graph::stop::Index_first_intermediate_stop];
      ASSERT_EQ(stop.index_,
                connection_graph::stop::Index_first_intermediate_stop);
      ASSERT_EQ(stop.alternative_infos_.size(), 1);
      {
        auto const& ic = stop.alternative_infos_[0];
        ASSERT_EQ(ic.journey_index_, 1);
        ASSERT_EQ(ic.next_stop_index_,
                  connection_graph::stop::Index_arrival_stop);
        // ASSERT_EQ(ic.interchange_time, 5);
      }
    }

    ASSERT_EQ(cg.journeys_.size(), 2);
    {
      auto const& j = cg.journeys_[0];
      ASSERT_EQ(j.stops.front().eva_no, "3333333");
      ASSERT_EQ(j.stops.back().eva_no, "2222222");
      ASSERT_EQ(j.stops.front().departure.timestamp, 1445238000);  // 07:00
      ASSERT_EQ(j.stops.back().arrival.timestamp, 1445238600);  // 07:10
      ASSERT_EQ(j.transports.front().train_nr, RE_D_L);
    }
    {
      auto const& j = cg.journeys_[1];
      ASSERT_EQ(j.stops.front().eva_no, "2222222");
      ASSERT_EQ(j.stops.back().eva_no, "1111111");
      ASSERT_EQ(j.stops.front().departure.timestamp, 1445238900);  // 07:15
      ASSERT_EQ(j.stops.back().arrival.timestamp, 1445239500);  // 07:10
      ASSERT_EQ(j.transports.front().train_nr, RE_L_F);
    }
  };

  search_cgs(msg->content<ReliableRoutingRequest const*>(),
             get_reliability_module(), 0,
             std::make_shared<simple_optimizer>(1, 1), test_cb);
  motis_instance_->run();
  ASSERT_TRUE(test_cb_called);
}

TEST_F(reliability_connection_graph_search, cache_journey) {
  detail::context c(get_reliability_module(), 0, callback(),
                    std::make_shared<simple_optimizer>(1, 1));
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
