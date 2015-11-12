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

class test_reliability2 : public test_schedule_setup {
public:
  test_reliability2()
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

class test_reliability7 : public test_schedule_setup {
public:
  test_reliability7()
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

TEST_F(test_reliability2, rating_request) {
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

TEST_F(test_reliability7, connection_tree) {
  system_tools::setup setup(schedule_.get());
  auto msg = flatbuffers_tools::to_connection_tree_request(
      DARMSTADT.name, DARMSTADT.eva, FRANKFURT.name, FRANKFURT.eva,
      (motis::time)(7 * 60), (motis::time)(7 * 60 + 1),
      std::make_tuple(19, 10, 2015), 3, 1);
  bool test_cb_called = false;

  auto test_cb = [&](motis::module::msg_ptr msg, boost::system::error_code e) {
    test_cb_called = true;
    ASSERT_EQ(nullptr, e);
    ASSERT_NE(nullptr, msg);
    std::ifstream f("modules/reliability/resources/json/reliable_search.json");
    std::string expected_str((std::istreambuf_iterator<char>(f)),
                             std::istreambuf_iterator<char>());
    ASSERT_EQ(expected_str, msg->to_json());
  };

  setup.dispatcher_.on_msg(msg, 0, test_cb);
  setup.ios_.run();

  ASSERT_TRUE(test_cb_called);
}

TEST_F(test_reliability7, reliable_connection_graph) {
  system_tools::setup setup(schedule_.get());
  auto msg = flatbuffers_tools::to_reliable_routing_request(
      DARMSTADT.name, DARMSTADT.eva, FRANKFURT.name, FRANKFURT.eva,
      (motis::time)(7 * 60), (motis::time)(7 * 60 + 1),
      std::make_tuple(19, 10, 2015), 1);
  bool test_cb_called = false;

  auto test_cb = [&](motis::module::msg_ptr msg, boost::system::error_code e) {
    test_cb_called = true;
    ASSERT_EQ(nullptr, e);
    ASSERT_NE(nullptr, msg);
    std::ifstream f("modules/reliability/resources/json/reliable_search.json");
    std::string expected_str((std::istreambuf_iterator<char>(f)),
                             std::istreambuf_iterator<char>());
    ASSERT_EQ(expected_str, msg->to_json());
  };

  setup.dispatcher_.on_msg(msg, 0, test_cb);
  setup.ios_.run();

  ASSERT_TRUE(test_cb_called);
}

}  // namespace reliability
}  // namespace motis
