#include "gtest/gtest.h"

#include <fstream>
#include <iostream>
#include <streambuf>
#include <string>
#include <vector>

#include "motis/core/common/date_time_util.h"

#include "motis/loader/loader.h"

#include "motis/module/dispatcher.h"

#include "motis/routing/routing.h"

#include "motis/reliability/rating/connection_rating.h"
#include "motis/reliability/reliability.h"
#include "motis/reliability/tools/flatbuffers/request_builder.h"

#include "include/schedules/schedule2.h"
#include "include/schedules/schedule7_cg.h"
#include "include/start_and_travel_test_distributions.h"
#include "include/test_schedule_setup.h"
#include "include/test_util.h"

using namespace motis::module;

namespace motis {
namespace reliability {

class reliability_test_rating : public test_motis_setup {
public:
  reliability_test_rating()
      : test_motis_setup(schedule2::PATH, schedule2::DATE) {}
};

class reliability_test_cg : public test_motis_setup {
public:
  reliability_test_cg()
      : test_motis_setup(schedule7_cg::PATH, schedule7_cg::DATE) {}

  void test_journey(Connection const* j, std::string const departure_eva,
                    std::string const arrival_eva, time_t const departure_time,
                    time_t const arrival_time, unsigned int const train_nr) {
    auto const first_stop = j->stops()->begin();
    auto const last_stop = (*j->stops())[j->stops()->size() - 1];
    auto const transport =
        reinterpret_cast<Transport const*>(j->transports()->begin()->move());
    ASSERT_EQ(departure_eva, first_stop->station()->id()->c_str());
    ASSERT_EQ(arrival_eva, last_stop->station()->id()->c_str());
    ASSERT_EQ(departure_time, first_stop->departure()->time());
    ASSERT_EQ(arrival_time, last_stop->arrival()->time());
    ASSERT_EQ(train_nr, transport->train_nr());
  }

  void test_alternative_rating(AlternativeRating const* rating,
                               time_t const dep_begin, time_t const arr_begin,
                               float const dep_first_prob,
                               float const arr_first_prob, float const sum) {
    ASSERT_EQ(dep_begin, rating->departure_distribution()->begin_time());
    ASSERT_FLOAT_EQ(dep_first_prob,
                    *rating->departure_distribution()->distribution()->begin());
    ASSERT_FLOAT_EQ(sum, rating->departure_distribution()->sum());
    ASSERT_EQ(arr_begin, rating->arrival_distribution()->begin_time());
    ASSERT_FLOAT_EQ(arr_first_prob,
                    *rating->arrival_distribution()->distribution()->begin());
    ASSERT_FLOAT_EQ(sum, rating->arrival_distribution()->sum());
  }

  void test_cg(ReliableRoutingResponse const* res) {
    ASSERT_EQ(1, res->connection_graphs()->size());
    auto cg = *res->connection_graphs()->begin();
    ASSERT_EQ(3, cg->stops()->size());

    {
      auto const stop = (*cg->stops())[0];
      ASSERT_EQ(0, stop->index());
      ASSERT_EQ(1, stop->alternatives()->size());
      ASSERT_EQ(0, stop->alternatives()->begin()->journey());
      ASSERT_EQ(2, stop->alternatives()->begin()->next_stop());
      test_alternative_rating(
          stop->alternatives()->begin()->rating(),
          1445230800 /* 10/19/2015, 7:00:00 AM GMT+2:00 DST */,
          1445231340 /* 10/19/2015, 7:09:00 AM GMT+2:00 DST */, 0.8, 0.08, 1.0);
    }
    {
      auto const stop = (*cg->stops())[1];
      ASSERT_EQ(1, stop->index());
      ASSERT_EQ(0, stop->alternatives()->size());
    }
    {
      auto const stop = (*cg->stops())[2];
      ASSERT_EQ(2, stop->index());
      ASSERT_EQ(3, stop->alternatives()->size());
      {
        auto const alternative = (*stop->alternatives())[0];
        ASSERT_EQ(1, alternative->journey());
        ASSERT_EQ(1, alternative->next_stop());
        test_alternative_rating(
            stop->alternatives()->begin()->rating(),
            1445231700 /* 10/19/2015, 7:15:00 AM GMT+2:00 DST */,
            1445232240 /* 10/19/2015, 7:24:00 AM GMT+2:00 DST */, 0.592, 0.0592,
            0.74);
      }
      {
        auto const alternative = (*stop->alternatives())[1];
        ASSERT_EQ(2, alternative->journey());
        ASSERT_EQ(1, alternative->next_stop());
        test_alternative_rating(
            (*stop->alternatives())[1]->rating(),
            1445231760 /* 10/19/2015, 7:16:00 AM GMT+2:00 DST */,
            1445232780 /* 10/19/2015, 7:33:00 AM GMT+2:00 DST */, 0.192, 0.0192,
            0.24);
      }
      {
        auto const alternative = (*stop->alternatives())[2];
        ASSERT_EQ(3, alternative->journey());
        ASSERT_EQ(1, alternative->next_stop());
        test_alternative_rating(
            (*stop->alternatives())[2]->rating(),
            1445231820 /* 10/19/2015, 7:17:00 AM GMT+2:00 DST */,
            1445233140 /* 10/19/2015, 7:39:00 AM GMT+2:00 DST */, 0.016, 0.0016,
            0.02);
      }
    }

    ASSERT_EQ(4, cg->journeys()->size());
    test_journey((*cg->journeys())[0], "3333333", "2222222",
                 1445230800 /* 10/19/2015, 7:00:00 AM GMT+2:00 DST */,
                 1445231400 /* 10/19/2015, 7:10:00 AM GMT+2:00 DST */,
                 schedule7_cg::RE_D_L);
    test_journey((*cg->journeys())[1], "2222222", "1111111",
                 1445231700 /* 10/19/2015, 7:15:00 AM GMT+2:00 DST */,
                 1445232300 /* 10/19/2015, 7:25:00 AM GMT+2:00 DST */,
                 schedule7_cg::RE_L_F);
    test_journey((*cg->journeys())[2], "2222222", "1111111",
                 1445231760 /* 10/19/2015, 7:16:00 AM GMT+2:00 DST */,
                 1445232840 /* 10/19/2015, 7:34:00 AM GMT+2:00 DST */,
                 schedule7_cg::S_L_F);
    test_journey((*cg->journeys())[3], "2222222", "1111111",
                 1445231820 /* 10/19/2015, 7:17:00 AM GMT+2:00 DST */,
                 1445233200 /* 10/19/2015, 7:40:00 AM GMT+2:00 DST */,
                 schedule7_cg::IC_L_F);
  }
};

TEST_F(reliability_test_rating, rating_request) {
  auto const req =
      request_builder()
          .add_pretrip_start(schedule2::STUTTGART.name_,
                             schedule2::STUTTGART.eva_,
                             test_util::hhmm_to_unixtime(get_schedule(), 1132),
                             test_util::hhmm_to_unixtime(get_schedule(), 1132))
          .add_destination(schedule2::KASSEL.name_, schedule2::KASSEL.eva_)
          .build_rating_request(false, false, false);
  auto const res = call(req);
  auto const response = motis_content(ReliabilityRatingResponse, res);

  ASSERT_EQ(1, response->response()->connections()->size());

  ASSERT_NE(response->ratings(), nullptr);
  ASSERT_EQ(response->ratings()->size(), 1);
  auto const& rating = response->ratings()->begin();
  ASSERT_TRUE(equal(rating->connection_rating(), 1.0));
  auto const& rating_elements = *rating->rating_elements();
  ASSERT_EQ(rating_elements.size(), 2);
  ASSERT_TRUE(equal(rating_elements[0]->dep_distribution()->sum(), 1.0));
  ASSERT_TRUE(equal(rating_elements[0]->arr_distribution()->sum(), 1.0));
  ASSERT_TRUE(equal(rating_elements[1]->dep_distribution()->sum(), 1.0));
  ASSERT_TRUE(equal(rating_elements[1]->arr_distribution()->sum(), 1.0));
  ASSERT_NE((*rating->rating_elements())[0]->range(), nullptr);
  ASSERT_NE((*rating->rating_elements())[1]->range(), nullptr);

  ASSERT_NE(response->simple_ratings(), nullptr);
  ASSERT_EQ(response->simple_ratings()->size(), 1);
  auto const simple_rating = response->simple_ratings()->begin();
  ASSERT_TRUE(equal(simple_rating->connection_rating(), 0.995 * 0.995));
  ASSERT_EQ(simple_rating->rating_elements()->size(), 2);
  ASSERT_NE((*simple_rating->rating_elements())[0]->range(), nullptr);
  ASSERT_NE((*simple_rating->rating_elements())[1]->range(), nullptr);
  ASSERT_EQ((*simple_rating->rating_elements())[0]->ratings()->size(), 1);
  ASSERT_EQ((*simple_rating->rating_elements())[1]->ratings()->size(), 2);
}

TEST_F(reliability_test_cg, connection_tree) {
  auto const req =
      request_builder()
          .add_pretrip_start(schedule7_cg::DARMSTADT.name_,
                             schedule7_cg::DARMSTADT.eva_,
                             test_util::hhmm_to_unixtime(get_schedule(), 700),
                             test_util::hhmm_to_unixtime(get_schedule(), 701))
          .add_destination(schedule7_cg::FRANKFURT.name_,
                           schedule7_cg::FRANKFURT.eva_)
          .build_connection_tree_request(3, 1);
  auto const res = call(req);
  test_cg(motis_content(ReliableRoutingResponse, res));
}

TEST_F(reliability_test_cg, reliable_connection_graph) {
  auto const req =
      request_builder()
          .add_pretrip_start(schedule7_cg::DARMSTADT.name_,
                             schedule7_cg::DARMSTADT.eva_,
                             test_util::hhmm_to_unixtime(get_schedule(), 700),
                             test_util::hhmm_to_unixtime(get_schedule(), 701))
          .add_destination(schedule7_cg::FRANKFURT.name_,
                           schedule7_cg::FRANKFURT.eva_)
          .build_reliable_search_request(1, false, false, false);
  auto const res = call(req);
  test_cg(motis_content(ReliableRoutingResponse, res));
}

}  // namespace reliability
}  // namespace motis
