#include "gtest/gtest.h"

#include <iostream>
#include <vector>

#include "motis/core/common/date_time_util.h"

#include "motis/reliability/reliability.h"
#include "motis/reliability/search/cg_optimizer.h"
#include "motis/reliability/search/cg_search_context.h"
#include "motis/reliability/search/connection_graph.h"
#include "motis/reliability/search/connection_graph_search.h"
#include "motis/reliability/tools/flatbuffers/request_builder.h"

#include "motis/test/motis_instance_helper.h"

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

  void test_cg(std::vector<std::shared_ptr<connection_graph> > const cgs) {
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
      ASSERT_EQ(j.stops_.front().eva_no_, "3333333");
      ASSERT_EQ(j.stops_.back().eva_no_, "2222222");
      ASSERT_EQ(j.stops_.front().departure_.timestamp_, 1445238000);  // 07:00
      ASSERT_EQ(j.stops_.back().arrival_.timestamp_, 1445238600);  // 07:10
      ASSERT_EQ(j.transports_.front().train_nr_, RE_D_L);
    }
    {
      auto const& j = cg.journeys_[1];
      ASSERT_EQ(j.stops_.front().eva_no_, "2222222");
      ASSERT_EQ(j.stops_.back().eva_no_, "1111111");
      ASSERT_EQ(j.stops_.front().departure_.timestamp_, 1445238900);  // 07:15
      ASSERT_EQ(j.stops_.back().arrival_.timestamp_, 1445239500);  // 07:25
      ASSERT_EQ(j.transports_.front().train_nr_, RE_L_F);
    }
    {
      auto const& j = cg.journeys_[2];
      ASSERT_EQ(j.stops_.front().eva_no_, "2222222");
      ASSERT_EQ(j.stops_.back().eva_no_, "1111111");
      ASSERT_EQ(j.stops_.front().departure_.timestamp_, 1445238960);  // 07:16
      ASSERT_EQ(j.stops_.back().arrival_.timestamp_, 1445240040);  // 07:34
      ASSERT_EQ(j.transports_.front().train_nr_, S_L_F);
    }
    {
      auto const& j = cg.journeys_[3];
      ASSERT_EQ(j.stops_.front().eva_no_, "2222222");
      ASSERT_EQ(j.stops_.back().eva_no_, "1111111");
      ASSERT_EQ(j.stops_.front().departure_.timestamp_, 1445239020);  // 07:17
      ASSERT_EQ(j.stops_.back().arrival_.timestamp_, 1445240400);  // 07:40
      ASSERT_EQ(j.transports_.front().train_nr_, IC_L_F);
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
  test_cg(search_cgs(
      motis_content(ReliableRoutingRequest, msg), *reliability_context_,
      std::make_shared<connection_graph_search::simple_optimizer>(3, 1)));
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
  test_cg(search_cgs(motis_content(ReliableRoutingRequest, msg),
                     *reliability_context_,
                     std::make_shared<reliable_cg_optimizer>(1)));
}

/* search for alternatives at stops_ not necessary
 * (base connection is optimal) */
TEST_F(reliability_connection_graph_search, reliable_routing_request2) {
  auto msg = flatbuffers::request_builder::request_builder()
                 .add_station(DARMSTADT.name, DARMSTADT.eva)
                 .add_station(FRANKFURT.name, FRANKFURT.eva)
                 .set_interval(std::make_tuple(19, 10, 2015),
                               (motis::time)(7 * 60), (motis::time)(7 * 60 + 1))
                 .build_connection_tree_request(1, 1);
  auto const cgs = search_cgs(motis_content(ReliableRoutingRequest, msg),
                              *reliability_context_,
                              std::make_shared<simple_optimizer>(1, 1));

  ASSERT_EQ(cgs.size(), 1);
  auto const cg = *cgs.front();

  ASSERT_EQ(cg.stops_.size(), 3);
  {
    auto const& stop = cg.stops_[connection_graph::stop::Index_departure_stop];
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
    ASSERT_EQ(j.stops_.front().eva_no_, "3333333");
    ASSERT_EQ(j.stops_.back().eva_no_, "2222222");
    ASSERT_EQ(j.stops_.front().departure_.timestamp_, 1445238000);  // 07:00
    ASSERT_EQ(j.stops_.back().arrival_.timestamp_, 1445238600);  // 07:10
    ASSERT_EQ(j.transports_.front().train_nr_, RE_D_L);
  }
  {
    auto const& j = cg.journeys_[1];
    ASSERT_EQ(j.stops_.front().eva_no_, "2222222");
    ASSERT_EQ(j.stops_.back().eva_no_, "1111111");
    ASSERT_EQ(j.stops_.front().departure_.timestamp_, 1445238900);  // 07:15
    ASSERT_EQ(j.stops_.back().arrival_.timestamp_, 1445239500);  // 07:10
    ASSERT_EQ(j.transports_.front().train_nr_, RE_L_F);
  }
}

TEST_F(reliability_connection_graph_search, cache_journey) {
  detail::context c(*reliability_context_,
                    std::make_shared<simple_optimizer>(1, 1));
  using key = detail::context::journey_cache_key;
  {
    journey j;
    j.duration_ = 10;
    c.journey_cache_[key("A", 0)] = j;
  }
  {
    journey j;
    j.duration_ = 20;
    c.journey_cache_[key("B", 1)] = j;
    ASSERT_EQ(c.journey_cache_.end(), c.journey_cache_.find(key("B", 1)));
  }
  {
    journey j;
    j.duration_ = 30;
    c.journey_cache_[key("A", 1)] = j;
  }
  {
    journey j;
    j.duration_ = 40;
    c.journey_cache_[key("B", 0)] = j;
  }

  ASSERT_EQ(10, c.journey_cache_.at(key("A", 0)).duration_);
  ASSERT_EQ(30, c.journey_cache_.at(key("A", 1)).duration_);
  ASSERT_EQ(40, c.journey_cache_.at(key("B", 0)).duration_);
  ASSERT_EQ(20, c.journey_cache_.at(key("B", 1)).duration_);

  ASSERT_EQ(c.journey_cache_.end(), c.journey_cache_.find(key("A", 3)));
  ASSERT_EQ(c.journey_cache_.end(), c.journey_cache_.find(key("A", 3)));
  ASSERT_EQ(c.journey_cache_.end(), c.journey_cache_.find(key("C", 0)));
}

}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
