#include "gtest/gtest.h"

#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include <vector>

#include "motis/core/common/date_util.h"

#include "motis/loader/loader.h"

#include "motis/module/dispatcher.h"

#include "motis/routing/routing.h"

#include "motis/reliability/reliability.h"
#include "motis/reliability/rating/connection_rating.h"
#include "motis/reliability/tools/flatbuffers_tools.h"
#include "motis/reliability/tools/system.h"

#include "include/start_and_travel_test_distributions.h"
#include "include/test_schedule_setup.h"

using namespace motis::module;

namespace motis {
namespace reliability {

class reliability_test_rating : public test_schedule_setup {
public:
  reliability_test_rating()
      : test_schedule_setup("modules/reliability/resources/schedule2/",
                            to_unix_time(2015, 9, 28),
                            to_unix_time(2015, 9, 29)) {}

  schedule_station const ERLANGEN = {"Erlangen", "0953067"};
  schedule_station const FRANKFURT = {"Frankfurt", "5744986"};
  schedule_station const KASSEL = {"Kassel", "6380201"};
  schedule_station const STUTTGART = {"Stuttgart", "7309882"};
  short const ICE_S_E = 5;  // 11:32 --> 12:32
  short const ICE_E_K = 7;  // 12:45 --> 14:15
};

class reliability_test_cg : public test_schedule_setup {
public:
  reliability_test_cg()
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

  void test_journey(routing::Connection const* j,
                    std::string const departure_eva,
                    std::string const arrival_eva, time_t const departure_time,
                    time_t const arrival_time, unsigned int const train_nr) {
    auto const first_stop = j->stops()->begin();
    auto const last_stop = (*j->stops())[j->stops()->size() - 1];
    auto const transport =
        ((routing::Transport const*)j->transports()->begin()->move());
    ASSERT_EQ(departure_eva, first_stop->eva_nr()->c_str());
    ASSERT_EQ(arrival_eva, last_stop->eva_nr()->c_str());
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

  void test_cg(motis::module::msg_ptr msg, boost::system::error_code e) {
    ASSERT_EQ(nullptr, e);
    ASSERT_NE(nullptr, msg);

    ASSERT_EQ(MsgContent_ReliableRoutingResponse, msg->content_type());
    auto res = msg->content<ReliableRoutingResponse const*>();

    ASSERT_EQ(1, res->connection_graphs()->size());
    auto cg = *res->connection_graphs()->begin();
    ASSERT_EQ(3, cg->stops()->size());

    {
      auto const stop = (*cg->stops())[0];
      ASSERT_EQ(0, stop->index());
      ASSERT_EQ(1, stop->alternatives()->size());
      ASSERT_EQ(0, stop->alternatives()->begin()->journey());
      ASSERT_EQ(2, stop->alternatives()->begin()->next_stop());
      test_alternative_rating(stop->alternatives()->begin()->rating(),
                              1445238000, 1445238540, 0.8, 0.08, 1.0);
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
        test_alternative_rating(stop->alternatives()->begin()->rating(),
                                1445238900, 1445239440, 0.592, 0.0592, 0.74);
      }
      {
        auto const alternative = (*stop->alternatives())[1];
        ASSERT_EQ(2, alternative->journey());
        ASSERT_EQ(1, alternative->next_stop());
        test_alternative_rating((*stop->alternatives())[1]->rating(),
                                1445238960, 1445239980, 0.192, 0.0192, 0.24);
      }
      {
        auto const alternative = (*stop->alternatives())[2];
        ASSERT_EQ(3, alternative->journey());
        ASSERT_EQ(1, alternative->next_stop());
        test_alternative_rating((*stop->alternatives())[2]->rating(),
                                1445239020, 1445240340, 0.016, 0.0016, 0.02);
      }
    }

    ASSERT_EQ(4, cg->journeys()->size());
    test_journey((*cg->journeys())[0], "3333333", "2222222",
                 1445238000 /* 07:00 */, 1445238600 /* 07:10 */, RE_D_L);
    test_journey((*cg->journeys())[1], "2222222", "1111111",
                 1445238900 /* 07:15 */, 1445239500 /* 07:25 */, RE_L_F);
    test_journey((*cg->journeys())[2], "2222222", "1111111",
                 1445238960 /* 07:16 */, 1445240040 /* 07:34 */, S_L_F);
    test_journey((*cg->journeys())[3], "2222222", "1111111",
                 1445239020 /* 07:17 */, 1445240400 /* 07:40 */, IC_L_F);
  }
};

TEST_F(reliability_test_rating, rating_request) {
  system_tools::setup setup(schedule_.get());
  auto msg = flatbuffers_tools::to_rating_request(
      STUTTGART.name, STUTTGART.eva, KASSEL.name, KASSEL.eva,
      (motis::time)(11 * 60 + 27),
      (motis::time)(
          11 * 60 +
          27) /* regard interchange time at the beginning of the journey */,
      std::make_tuple(28, 9, 2015));
  bool test_cb_called = false;

  auto test_cb = [&](motis::module::msg_ptr msg, boost::system::error_code e) {
    test_cb_called = true;
    ASSERT_EQ(e, nullptr);
    auto response = msg->content<ReliabilityRatingResponse const*>();
    ASSERT_EQ(1, response->response()->connections()->size());

    ASSERT_NE(response->ratings(), nullptr);
    ASSERT_EQ(response->ratings()->size(), 1);
    auto const& rating = response->ratings()->begin();
    ASSERT_DOUBLE_EQ(rating->connection_rating(), 1.0);
    ASSERT_EQ(rating->rating_elements()->size(), 2);
    ASSERT_DOUBLE_EQ((*rating->rating_elements())[0]->dep_distribution()->sum(),
                     1.0);
    ASSERT_DOUBLE_EQ((*rating->rating_elements())[0]->arr_distribution()->sum(),
                     1.0);
    ASSERT_DOUBLE_EQ((*rating->rating_elements())[1]->dep_distribution()->sum(),
                     1.0);
    ASSERT_DOUBLE_EQ((*rating->rating_elements())[1]->arr_distribution()->sum(),
                     1.0);
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
  };

  setup.dispatcher_.on_msg(msg, 0, test_cb);
  setup.ios_.run();

  ASSERT_TRUE(test_cb_called);
}

TEST_F(reliability_test_cg, connection_tree) {
  system_tools::setup setup(schedule_.get());
  auto msg = flatbuffers_tools::to_connection_tree_request(
      DARMSTADT.name, DARMSTADT.eva, FRANKFURT.name, FRANKFURT.eva,
      (motis::time)(7 * 60), (motis::time)(7 * 60 + 1),
      std::make_tuple(19, 10, 2015), 3, 1);
  bool test_cb_called = false;

  auto test_cb = [&](motis::module::msg_ptr msg, boost::system::error_code e) {
    test_cb_called = true;
    test_cg(msg, e);
  };

  setup.dispatcher_.on_msg(msg, 0, test_cb);
  setup.ios_.run();

  ASSERT_TRUE(test_cb_called);
}

TEST_F(reliability_test_cg, reliable_connection_graph) {
  system_tools::setup setup(schedule_.get());
  auto msg = flatbuffers_tools::to_reliable_routing_request(
      DARMSTADT.name, DARMSTADT.eva, FRANKFURT.name, FRANKFURT.eva,
      (motis::time)(7 * 60), (motis::time)(7 * 60 + 1),
      std::make_tuple(19, 10, 2015), 1);
  bool test_cb_called = false;

  auto test_cb = [&](motis::module::msg_ptr msg, boost::system::error_code e) {
    test_cb_called = true;
    test_cg(msg, e);
  };

  setup.dispatcher_.on_msg(msg, 0, test_cb);
  setup.ios_.run();

  ASSERT_TRUE(test_cb_called);
}

}  // namespace reliability
}  // namespace motis
