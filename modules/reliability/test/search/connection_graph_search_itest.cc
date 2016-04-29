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
#include "../include/test_util.h"

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
  schedule_station const PFUNGSTADT = {"Pfungstadt", "5420132"};
  short const RE_D_L = 1;  // 07:00 --> 07:10
  short const RE_L_F = 2;  // 07:15 --> 07:25
  short const S_L_F = 3;  // 07:16 --> 07:34
  short const IC_L_F = 4;  // 07:17 --> 07:40

  short const RE_P_D = 5;  // 06:40 --> 06:50
  short const IC_DA_F = 6;  // 06:50 --> 07:10

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
      ASSERT_EQ(1445230800 /* 10/19/2015, 7:00:00 AM GMT+2:00 DST */,
                j.stops_.front().departure_.timestamp_);
      ASSERT_EQ(1445231400 /* 10/19/2015, 7:10:00 AM GMT+2:00 DST */,
                j.stops_.back().arrival_.timestamp_);
      ASSERT_EQ(j.transports_.front().train_nr_, RE_D_L);
    }
    {
      auto const& j = cg.journeys_[1];
      ASSERT_EQ(j.stops_.front().eva_no_, "2222222");
      ASSERT_EQ(j.stops_.back().eva_no_, "1111111");
      ASSERT_EQ(1445231700 /* 10/19/2015, 7:15:00 AM GMT+2:00 DST */,
                j.stops_.front().departure_.timestamp_);
      ASSERT_EQ(1445232300 /* 10/19/2015, 7:25:00 AM GMT+2:00 DST */,
                j.stops_.back().arrival_.timestamp_);
      ASSERT_EQ(j.transports_.front().train_nr_, RE_L_F);
    }
    {
      auto const& j = cg.journeys_[2];
      ASSERT_EQ(j.stops_.front().eva_no_, "2222222");
      ASSERT_EQ(j.stops_.back().eva_no_, "1111111");
      ASSERT_EQ(1445231760 /* 10/19/2015, 7:16:00 AM GMT+2:00 DST */,
                j.stops_.front().departure_.timestamp_);
      ASSERT_EQ(1445232840 /* 10/19/2015, 7:34:00 AM GMT+2:00 DST */,
                j.stops_.back().arrival_.timestamp_);
      ASSERT_EQ(j.transports_.front().train_nr_, S_L_F);
    }
    {
      auto const& j = cg.journeys_[3];
      ASSERT_EQ(j.stops_.front().eva_no_, "2222222");
      ASSERT_EQ(j.stops_.back().eva_no_, "1111111");
      ASSERT_EQ(1445231820 /* 10/19/2015, 7:17:00 AM GMT+2:00 DST */,
                j.stops_.front().departure_.timestamp_);
      ASSERT_EQ(1445233200 /* 10/19/2015, 7:40:00 AM GMT+2:00 DST */,
                j.stops_.back().arrival_.timestamp_);
      ASSERT_EQ(j.transports_.front().train_nr_, IC_L_F);
    }
  }
};

/* optimize connection graph alternatives depending on distributions! */
TEST_F(reliability_connection_graph_search,
       reliable_routing_request_optimization) {
  auto const msg =
      flatbuffers::request_builder()
          .add_station(DARMSTADT.name, DARMSTADT.eva)
          .add_station(FRANKFURT.name, FRANKFURT.eva)
          .set_interval(test_util::hhmm_to_unixtime(get_schedule(), 700),
                        test_util::hhmm_to_unixtime(get_schedule(), 700))
          .build_reliable_search_request(1);
  test_cg(motis_instance_->run([&]() {
    return search_cgs(motis_content(ReliableRoutingRequest, msg),
                      *reliability_context_,
                      std::make_shared<reliable_cg_optimizer>(1));
  }));
}

TEST_F(reliability_connection_graph_search,
       connection_tree_three_alternatives) {
  auto const msg =
      flatbuffers::request_builder()
          .add_station(DARMSTADT.name, DARMSTADT.eva)
          .add_station(FRANKFURT.name, FRANKFURT.eva)
          .set_interval(test_util::hhmm_to_unixtime(get_schedule(), 700),
                        test_util::hhmm_to_unixtime(get_schedule(), 700))
          .build_connection_tree_request(3, 1);

  test_cg(motis_instance_->run([&]() {
    return search_cgs(
        motis_content(ReliableRoutingRequest, msg), *reliability_context_,
        std::make_shared<connection_graph_search::simple_optimizer>(3, 1));
  }));
}

/* search for alternatives at stops_ not necessary
 * (base connection is optimal) */
TEST_F(reliability_connection_graph_search, connection_three_one_alternative) {
  auto msg = flatbuffers::request_builder()
                 .add_station(DARMSTADT.name, DARMSTADT.eva)
                 .add_station(FRANKFURT.name, FRANKFURT.eva)
                 .set_interval(test_util::hhmm_to_unixtime(get_schedule(), 700),
                               test_util::hhmm_to_unixtime(get_schedule(), 700))
                 .build_connection_tree_request(1, 1);
  auto const cgs = motis_instance_->run([&]() {
    return search_cgs(
        motis_content(ReliableRoutingRequest, msg), *reliability_context_,
        std::make_shared<connection_graph_search::simple_optimizer>(1, 1));
  });

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
    ASSERT_EQ(1445230800 /* 10/19/2015, 7:00:00 AM GMT+2:00 DST */,
              j.stops_.front().departure_.timestamp_);
    ASSERT_EQ(1445231400 /* 10/19/2015, 7:10:00 AM GMT+2:00 DST */,
              j.stops_.back().arrival_.timestamp_);
    ASSERT_EQ(j.transports_.front().train_nr_, RE_D_L);
  }
  {
    auto const& j = cg.journeys_[1];
    ASSERT_EQ(j.stops_.front().eva_no_, "2222222");
    ASSERT_EQ(j.stops_.back().eva_no_, "1111111");
    ASSERT_EQ(1445231700 /* 10/19/2015, 7:15:00 AM GMT+2:00 DST */,
              j.stops_.front().departure_.timestamp_);
    ASSERT_EQ(1445232300 /* 10/19/2015, 7:25:00 AM GMT+2:00 DST */,
              j.stops_.back().arrival_.timestamp_);
    ASSERT_EQ(j.transports_.front().train_nr_, RE_L_F);
  }
}

/* This test covers two cases:
 * First, for stop in Darmstadt, three alternatives are required,
 * but only 2 exist in the database (the stop should be delivered
 * with 2 alternatives).
 * Second, the second alternative in Darmstadt has an interchange
 * in Langen which also requires further alternatives (three in total).
 */
TEST_F(reliability_connection_graph_search,
       alternative_requires_further_alternatives) {
  auto msg = flatbuffers::request_builder()
                 .add_station(PFUNGSTADT.name, PFUNGSTADT.eva)
                 .add_station(FRANKFURT.name, FRANKFURT.eva)
                 .set_interval(test_util::hhmm_to_unixtime(get_schedule(), 630),
                               test_util::hhmm_to_unixtime(get_schedule(), 630))
                 .build_connection_tree_request(3, 1);
  auto const cgs = motis_instance_->run([&]() {
    return search_cgs(
        motis_content(ReliableRoutingRequest, msg), *reliability_context_,
        std::make_shared<connection_graph_search::simple_optimizer>(3, 1));
  });

  ASSERT_EQ(cgs.size(), 1);
  auto const cg = *cgs.front();

  ASSERT_EQ(6, cg.journeys_.size());
  ASSERT_EQ(4, cg.stops_.size());

  // Pfungstadt
  ASSERT_EQ(1, cg.stops_.front().alternative_infos_.size());
  ASSERT_EQ(1, cg.stops_.front().alternative_infos_.front().next_stop_index_);
  {
    // Darmstadt
    auto const& stop = cg.stops_[1];
    ASSERT_EQ(2, stop.alternative_infos_.size());
    {
      ASSERT_EQ(3 /* Frankfurt */, stop.alternative_infos_[0].next_stop_index_);
      auto const& alternative =
          cg.journeys_[stop.alternative_infos_[0].journey_index_];
      ASSERT_EQ(DARMSTADT.eva, alternative.stops_.front().eva_no_);
      ASSERT_EQ(FRANKFURT.eva, alternative.stops_.back().eva_no_);
    }
    {
      ASSERT_EQ(3 /* Langen */, stop.alternative_infos_[0].next_stop_index_);
      auto const& alternative =
          cg.journeys_[stop.alternative_infos_[1].journey_index_];
      ASSERT_EQ(DARMSTADT.eva, alternative.stops_.front().eva_no_);
      ASSERT_EQ(LANGEN.eva, alternative.stops_.back().eva_no_);
    }
  }
  // Langen
  ASSERT_EQ(3, cg.stops_[2].alternative_infos_.size());
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
  }
  {
    ASSERT_EQ(c.journey_cache_.end(), c.journey_cache_.find(key("A", 1)));
    journey j;
    j.duration_ = 30;
    c.journey_cache_[key("A", 1)] = j;
    ASSERT_NE(c.journey_cache_.end(), c.journey_cache_.find(key("A", 1)));
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

  ASSERT_EQ(c.journey_cache_.end(), c.journey_cache_.find(key("A", 2)));
  ASSERT_EQ(c.journey_cache_.end(), c.journey_cache_.find(key("B", 2)));
  ASSERT_EQ(c.journey_cache_.end(), c.journey_cache_.find(key("C", 0)));
}

}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
