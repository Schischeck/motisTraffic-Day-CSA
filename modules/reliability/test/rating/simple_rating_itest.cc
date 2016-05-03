#include "gtest/gtest.h"

#include "motis/core/common/date_time_util.h"
#include "motis/core/journey/journey.h"
#include "motis/core/journey/message_to_journeys.h"

#include "motis/loader/loader.h"

#include "motis/routing/routing.h"

#include "motis/reliability/rating/simple_rating.h"
#include "motis/reliability/tools/flatbuffers/request_builder.h"

#include "motis/test/motis_instance_helper.h"

#include "../include/start_and_travel_test_distributions.h"
#include "../include/test_schedule_setup.h"
#include "../include/test_util.h"

namespace motis {
namespace reliability {
namespace rating {
namespace simple_rating {

class reliability_simple_rating2 : public test_motis_setup {
public:
  reliability_simple_rating2()
      : test_motis_setup("modules/reliability/resources/schedule2/",
                         "20150928") {}
  schedule_station const ERLANGEN = {"Erlangen", "0953067"};
  schedule_station const FRANKFURT = {"Frankfurt", "5744986"};
  schedule_station const KASSEL = {"Kassel", "6380201"};
  schedule_station const STUTTGART = {"Stuttgart", "7309882"};
  short const ICE_S_E = 5;  // 11:32 --> 12:32
  short const ICE_E_K = 7;  // 12:45 --> 14:15
};
class reliability_simple_rating5 : public test_motis_setup {
public:
  reliability_simple_rating5()
      : test_motis_setup("modules/reliability/resources/schedule5/",
                         "20151019") {}
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
TEST_F(reliability_simple_rating2, simple_rate) {
  auto req_msg =
      flatbuffers::request_builder()
          .add_pretrip_start(STUTTGART.name, STUTTGART.eva,
                             test_util::hhmm_to_unixtime(get_schedule(), 1132),
                             test_util::hhmm_to_unixtime(get_schedule(), 1132))
          .add_destination(KASSEL.name, KASSEL.eva)
          .build_routing_request();
  auto msg = test::call(motis_instance_, req_msg);
  using routing::RoutingResponse;
  auto const journeys =
      message_to_journeys(motis_content(RoutingResponse, msg));

  ASSERT_EQ(1, journeys.size());
  start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                        {0.1, 0.8, 0.1}, -1);

  simple_connection_rating rating;
  rate(rating, journeys.front(), get_schedule(), s_t_distributions);
  ASSERT_TRUE(rating.ratings_elements_.size() == 2);
  ASSERT_EQ(0, rating.ratings_elements_[0].from_);
  ASSERT_TRUE(rating.ratings_elements_[0].to_ == 1);
  ASSERT_TRUE(rating.ratings_elements_[1].from_ == 1);
  ASSERT_TRUE(rating.ratings_elements_[1].to_ == 2);
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
  ASSERT_TRUE(equal(1.0, rating.ratings_elements_[1].ratings_[1].second));

  ASSERT_TRUE(equal(rating.connection_rating_, 0.995 * 0.995));
}

/* Mannheim to Darmstadt with RE_M_B_D (interchange in Darmstadt),
 * Darmstadt to Giessen with RE_D_F_G (interchange in Giessen), and
 * Giessen to Marburg with RE_G_M */
TEST_F(reliability_simple_rating5, simple_rate2) {
  auto req_msg =
      flatbuffers::request_builder()
          .add_pretrip_start(MANNHEIM.name, MANNHEIM.eva,
                             test_util::hhmm_to_unixtime(get_schedule(), 700),
                             test_util::hhmm_to_unixtime(get_schedule(), 700))
          .add_destination(MARBURG.name, MARBURG.eva)
          .build_routing_request();
  auto msg = test::call(motis_instance_, req_msg);
  using routing::RoutingResponse;
  auto const journeys =
      message_to_journeys(motis_content(RoutingResponse, msg));

  ASSERT_EQ(1, journeys.size());
  start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                        {0.1, 0.8, 0.1}, -1);
  simple_connection_rating rating;
  rate(rating, journeys.front(), get_schedule(), s_t_distributions);

  ASSERT_TRUE(rating.ratings_elements_.size() == 3);
  {
    auto const& rating_element = rating.ratings_elements_[0];
    ASSERT_TRUE(rating_element.from_ == 0);
    ASSERT_TRUE(rating_element.to_ == 2);
    ASSERT_TRUE(rating_element.ratings_.size() == 1);
    ASSERT_TRUE(rating_element.ratings_[0].first == rating_type::Cancellation);
    ASSERT_TRUE(equal(rating_element.ratings_[0].second, 0.995));
  }
  {
    auto const& rating_element = rating.ratings_elements_[1];
    ASSERT_TRUE(rating_element.from_ == 2);
    ASSERT_TRUE(rating_element.to_ == 4);
    ASSERT_TRUE(rating_element.ratings_.size() == 2);
    ASSERT_TRUE(rating_element.ratings_[0].first == rating_type::Cancellation);
    ASSERT_TRUE(equal(rating_element.ratings_[0].second, 0.995));
    ASSERT_TRUE(rating_element.ratings_[1].first == rating_type::Interchange);
    ASSERT_TRUE(equal(rating_element.ratings_[1].second, 0.9));
  }
  {
    auto const& rating_element = rating.ratings_elements_[2];
    ASSERT_TRUE(rating_element.from_ == 4);
    ASSERT_TRUE(rating_element.to_ == 5);
    ASSERT_TRUE(rating_element.ratings_.size() == 2);
    ASSERT_TRUE(rating_element.ratings_[0].first == rating_type::Cancellation);
    ASSERT_TRUE(equal(rating_element.ratings_[0].second, 0.995));
    ASSERT_TRUE(rating_element.ratings_[1].first == rating_type::Interchange);
    ASSERT_TRUE(equal(rating_element.ratings_[1].second, 1.0));
  }

  ASSERT_TRUE(equal(rating.connection_rating_, 0.995 * 0.995 * 0.995 * 0.9));
}

}  // namespace simple_rating
}  // namespace rating
}  // namespace reliability
}  // namespace motis
