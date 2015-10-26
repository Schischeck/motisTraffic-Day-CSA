#include "gtest/gtest.h"

#include <iostream>
#include <fstream>
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

using namespace motis;
using namespace motis::reliability;
using namespace motis::module;

class test_reliability : public test_schedule_setup {
public:
  test_reliability()
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

TEST_F(test_reliability, reliable_routing_request) {
  system_tools::setup setup(schedule_.get());
  auto msg = flatbuffers_tools::to_reliable_routing_request(
      STUTTGART.name, STUTTGART.eva, KASSEL.name, KASSEL.eva,
      (motis::time)(11 * 60 + 30), (motis::time)(11 * 60 + 35),
      std::make_tuple(28, 9, 2015));

  auto test_cb = [&](motis::module::msg_ptr msg, boost::system::error_code e) {
    std::ofstream os("json.txt");
    os << msg->to_json() << std::endl;
    auto response = msg->content<ReliableRoutingResponse const*>();
    ASSERT_EQ(response->response()->connections()->size(), 1);

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

  setup.dispatcher.on_msg(msg, 0, test_cb);
  setup.ios.run();
}
