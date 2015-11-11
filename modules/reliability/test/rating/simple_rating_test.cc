#include "gtest/gtest.h"

#include "motis/core/common/date_util.h"
#include "motis/core/common/journey.h"
#include "motis/core/common/journey_builder.h"

#include "motis/loader/loader.h"

#include "motis/routing/routing.h"

#include "motis/reliability/rating/simple_rating.h"
#include "motis/reliability/tools/flatbuffers_tools.h"
#include "motis/reliability/tools/system.h"

#include "../include/start_and_travel_test_distributions.h"
#include "../include/test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace rating {
namespace simple_rating {

class test_simple_rating2 : public test_schedule_setup {
public:
  test_simple_rating2()
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
class test_simple_rating5 : public test_schedule_setup {
public:
  test_simple_rating5()
      : test_schedule_setup("modules/reliability/resources/schedule5/",
                            to_unix_time(2015, 10, 19),
                            to_unix_time(2015, 10, 20)) {}
  schedule_station const DARMSTADT = {"Darmstadt", "1111111"};
  schedule_station const FRANKFURT = {"Frankfurt", "2222222"};
  schedule_station const GIESSEN = {"Giessen", "3333333"};
  schedule_station const MARBURG = {"Marburg", "4444444"};
  schedule_station const BENSHEIM = {"Bensheim", "5555555"};
  schedule_station const MANNHEIM = {"Mannheim", "6666666"};
  short const RE_M_B_D = 3;  // 07:00 --> 07:30, 07:31 --> 07:55
  short const RE_D_F_G = 1;  // 08:00 --> 08:20, 08:22 --> 09:00
  short const RE_G_M = 2;  // 09:10 --> 09:40
};

/* Stuttgart to Erlangen with ICE_S_E (interchange in Stuttgart) and
 * Erlangen to Kassel with ICE_E_K */
TEST_F(test_simple_rating2, simple_rate) {
  system_tools::setup setup(schedule_.get());
  auto msg = flatbuffers_tools::to_routing_request(
      STUTTGART.name, STUTTGART.eva, KASSEL.name, KASSEL.eva,
      (motis::time)(11 * 60 + 27),
      (motis::time)(
          11 * 60 +
          27) /* regard interchange time at the beginning of the journey */,
      std::make_tuple(28, 9, 2015), false);

  auto test_cb = [&](motis::module::msg_ptr msg, boost::system::error_code e) {
    auto const journeys = journey_builder::to_journeys(
        msg->content<routing::RoutingResponse const*>(), schedule_->categories);
    ASSERT_EQ(1, journeys.size());
    start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                          {0.1, 0.8, 0.1}, -1);

    simple_connection_rating rating;
    bool success =
        rate(rating, journeys.front(), *schedule_, s_t_distributions);
    ASSERT_TRUE(success);
    ASSERT_TRUE(rating.ratings_elements_.size() == 2);
    ASSERT_EQ(1, rating.ratings_elements_[0].from_);
    ASSERT_TRUE(rating.ratings_elements_[0].to_ == 2);
    ASSERT_TRUE(rating.ratings_elements_[1].from_ == 2);
    ASSERT_TRUE(rating.ratings_elements_[1].to_ == 3);
    ASSERT_TRUE(rating.ratings_elements_[0].ratings_.size() == 1);
    ASSERT_TRUE(rating.ratings_elements_[0].ratings_[0].first ==
                rating_type::Cancellation);
    ASSERT_TRUE(equal(rating.ratings_elements_[0].ratings_[0].second, 0.995));
    ASSERT_TRUE(rating.ratings_elements_[1].ratings_.size() == 2);
    ASSERT_TRUE(rating.ratings_elements_[1].ratings_[0].first ==
                rating_type::Cancellation);
    ASSERT_TRUE(equal(rating.ratings_elements_[1].ratings_[0].second, 0.995));
    ASSERT_TRUE(rating.ratings_elements_[1].ratings_[1].first ==
                rating_type::Interchange);
    ASSERT_TRUE(equal(rating.ratings_elements_[1].ratings_[1].second, 1.0));

    ASSERT_TRUE(equal(rating.connection_rating_, 0.995 * 0.995));
  };

  setup.dispatcher_.on_msg(msg, 0, test_cb);
  setup.ios_.run();
}

/* Mannheim to Darmstadt with RE_M_B_D (interchange in Darmstadt),
 * Darmstadt to Giessen with RE_D_F_G (interchange in Giessen), and
 * Giessen to Marburg with RE_G_M */
TEST_F(test_simple_rating5, simple_rate2) {
  system_tools::setup setup(schedule_.get());
  auto msg = flatbuffers_tools::to_routing_request(
      MANNHEIM.name, MANNHEIM.eva, MARBURG.name, MARBURG.eva,
      (motis::time)(7 * 60), (motis::time)(7 * 60 + 1),
      std::make_tuple(19, 10, 2015), false);

  auto test_cb = [&](motis::module::msg_ptr msg, boost::system::error_code e) {
    auto const journeys = journey_builder::to_journeys(
        msg->content<routing::RoutingResponse const*>(), schedule_->categories);
    ASSERT_EQ(1, journeys.size());
    start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                          {0.1, 0.8, 0.1}, -1);
    simple_connection_rating rating;
    bool success =
        rate(rating, journeys.front(), *schedule_, s_t_distributions);

    ASSERT_TRUE(success);
    ASSERT_TRUE(rating.ratings_elements_.size() == 3);
    {
      auto const& rating_element = rating.ratings_elements_[0];
      ASSERT_TRUE(rating_element.from_ == 1);
      ASSERT_TRUE(rating_element.to_ == 3);
      ASSERT_TRUE(rating_element.ratings_.size() == 1);
      ASSERT_TRUE(rating_element.ratings_[0].first ==
                  rating_type::Cancellation);
      ASSERT_TRUE(equal(rating_element.ratings_[0].second, 0.995));
    }
    {
      auto const& rating_element = rating.ratings_elements_[1];
      ASSERT_TRUE(rating_element.from_ == 3);
      ASSERT_TRUE(rating_element.to_ == 5);
      ASSERT_TRUE(rating_element.ratings_.size() == 2);
      ASSERT_TRUE(rating_element.ratings_[0].first ==
                  rating_type::Cancellation);
      ASSERT_TRUE(equal(rating_element.ratings_[0].second, 0.995));
      ASSERT_TRUE(rating_element.ratings_[1].first == rating_type::Interchange);
      ASSERT_TRUE(equal(rating_element.ratings_[1].second, 0.9));
    }
    {
      auto const& rating_element = rating.ratings_elements_[2];
      ASSERT_TRUE(rating_element.from_ == 5);
      ASSERT_TRUE(rating_element.to_ == 6);
      ASSERT_TRUE(rating_element.ratings_.size() == 2);
      ASSERT_TRUE(rating_element.ratings_[0].first ==
                  rating_type::Cancellation);
      ASSERT_TRUE(equal(rating_element.ratings_[0].second, 0.995));
      ASSERT_TRUE(rating_element.ratings_[1].first == rating_type::Interchange);
      ASSERT_TRUE(equal(rating_element.ratings_[1].second, 1.0));
    }

    ASSERT_TRUE(equal(rating.connection_rating_, 0.995 * 0.995 * 0.995 * 0.9));
  };

  setup.dispatcher_.on_msg(msg, 0, test_cb);
  setup.ios_.run();
}

}  // namespace simple_rating
}  // namespace rating
}  // namespace reliability
}  // namespace motis
