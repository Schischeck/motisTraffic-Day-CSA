#include "catch/catch.hpp"

#include "motis/core/common/date_util.h"

#include "motis/loader/loader.h"

#include "motis/routing/routing.h"

#include "motis/reliability/rating/simple_rating.h"
#include "motis/reliability/tools/flatbuffers_tools.h"
#include "motis/reliability/tools/system.h"

#include "include/start_and_travel_test_distributions.h"

using namespace motis;
using namespace motis::reliability;
using namespace motis::reliability::rating::simple_rating;

struct schedule_station {
  std::string name;
  std::string eva;
};

namespace schedule2 {
schedule_station const ERLANGEN = {"Erlangen", "0953067"};
schedule_station const FRANKFURT = {"Frankfurt", "5744986"};
schedule_station const KASSEL = {"Kassel", "6380201"};
schedule_station const STUTTGART = {"Stuttgart", "7309882"};
short const ICE_S_E = 5;  // 11:32 --> 12:32
short const ICE_E_K = 7;  // 12:45 --> 14:15
}
namespace schedule5 {
schedule_station const DARMSTADT = {"Darmstadt", "1111111"};
schedule_station const FRANKFURT = {"Frankfurt", "2222222"};
schedule_station const GIESSEN = {"Giessen", "3333333"};
schedule_station const MARBURG = {"Marburg", "4444444"};
schedule_station const BENSHEIM = {"Bensheim", "5555555"};
schedule_station const MANNHEIM = {"Mannheim", "6666666"};
short const RE_M_B_D = 3;  // 07:00 --> 07:30, 07:31 --> 07:55
short const RE_D_F_G = 1;  // 08:00 --> 08:20, 08:22 --> 09:00
short const RE_G_M = 2;  // 09:10 --> 09:40
}

namespace test_simple_rating {
auto schedule2 =
    loader::load_schedule("modules/reliability/resources/schedule2/",
                          to_unix_time(2015, 9, 28), to_unix_time(2015, 9, 29));
auto schedule5 = loader::load_schedule(
    "modules/reliability/resources/schedule5/", to_unix_time(2015, 10, 19),
    to_unix_time(2015, 10, 20));
}  // namespace test_simple_rating

/* Stuttgart to Erlangen with ICE_S_E (interchange in Stuttgart) and
 * Erlangen to Kassel with ICE_E_K */
TEST_CASE("simple_rate", "[simple_rating]") {
  system_tools::setup setup(test_simple_rating::schedule2.get());
  auto msg = flatbuffers_tools::to_routing_request(
      schedule2::STUTTGART.name, schedule2::STUTTGART.eva,
      schedule2::KASSEL.name, schedule2::KASSEL.eva,
      (motis::time)(11 * 60 + 30), (motis::time)(11 * 60 + 35),
      std::make_tuple(28, 9, 2015));

  auto test_cb = [&](motis::module::msg_ptr msg, boost::system::error_code e) {
    auto response = msg->content<routing::RoutingResponse const*>();
    start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                          {0.1, 0.8, 0.1}, -1);

    simple_connection_rating rating;
    bool success = rate(rating, *response->connections()->begin(),
                        *test_simple_rating::schedule2, s_t_distributions);
    REQUIRE(success);
    REQUIRE(rating.ratings_elements_.size() == 2);
    REQUIRE(rating.ratings_elements_[0].from_ == 1);
    REQUIRE(rating.ratings_elements_[0].to_ == 2);
    REQUIRE(rating.ratings_elements_[1].from_ == 2);
    REQUIRE(rating.ratings_elements_[1].to_ == 3);
    REQUIRE(rating.ratings_elements_[0].ratings_.size() == 1);
    REQUIRE(rating.ratings_elements_[0].ratings_[0].first ==
            rating_type::Cancellation);
    REQUIRE(equal(rating.ratings_elements_[0].ratings_[0].second, 0.995));
    REQUIRE(rating.ratings_elements_[1].ratings_.size() == 2);
    REQUIRE(rating.ratings_elements_[1].ratings_[0].first ==
            rating_type::Cancellation);
    REQUIRE(equal(rating.ratings_elements_[1].ratings_[0].second, 0.995));
    REQUIRE(rating.ratings_elements_[1].ratings_[1].first ==
            rating_type::Interchange);
    REQUIRE(equal(rating.ratings_elements_[1].ratings_[1].second, 1.0));

    REQUIRE(equal(rating.connection_rating_, 0.995 * 0.995));
  };

  setup.dispatcher.on_msg(msg, 0, test_cb);
  setup.ios.run();
}

/* Mannheim to Darmstadt with RE_M_B_D (interchange in Darmstadt),
 * Darmstadt to Giessen with RE_D_F_G (interchange in Giessen), and
 * Giessen to Marburg with RE_G_M */
TEST_CASE("simple_rate2", "[rate_public_transport]") {
  system_tools::setup setup(test_simple_rating::schedule5.get());
  auto msg = flatbuffers_tools::to_routing_request(
      schedule5::MANNHEIM.name, schedule5::MANNHEIM.eva,
      schedule5::MARBURG.name, schedule5::MARBURG.eva, (motis::time)(7 * 60),
      (motis::time)(7 * 60 + 1), std::make_tuple(19, 10, 2015));

  auto test_cb = [&](motis::module::msg_ptr msg, boost::system::error_code e) {
    auto response = msg->content<routing::RoutingResponse const*>();
    REQUIRE(response->connections()->size() == 1);
    start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                          {0.1, 0.8, 0.1}, -1);
    simple_connection_rating rating;
    bool success = rate(rating, *response->connections()->begin(),
                        *test_simple_rating::schedule5, s_t_distributions);

    REQUIRE(success);
    REQUIRE(rating.ratings_elements_.size() == 3);
    {
      auto const& rating_element = rating.ratings_elements_[0];
      REQUIRE(rating_element.from_ == 1);
      REQUIRE(rating_element.to_ == 3);
      REQUIRE(rating_element.ratings_.size() == 1);
      REQUIRE(rating_element.ratings_[0].first == rating_type::Cancellation);
      REQUIRE(equal(rating_element.ratings_[0].second, 0.995));
    }
    {
      auto const& rating_element = rating.ratings_elements_[1];
      REQUIRE(rating_element.from_ == 3);
      REQUIRE(rating_element.to_ == 5);
      REQUIRE(rating_element.ratings_.size() == 2);
      REQUIRE(rating_element.ratings_[0].first == rating_type::Cancellation);
      REQUIRE(equal(rating_element.ratings_[0].second, 0.995));
      REQUIRE(rating_element.ratings_[1].first == rating_type::Interchange);
      REQUIRE(equal(rating_element.ratings_[1].second, 0.9));
    }
    {
      auto const& rating_element = rating.ratings_elements_[2];
      REQUIRE(rating_element.from_ == 5);
      REQUIRE(rating_element.to_ == 6);
      REQUIRE(rating_element.ratings_.size() == 2);
      REQUIRE(rating_element.ratings_[0].first == rating_type::Cancellation);
      REQUIRE(equal(rating_element.ratings_[0].second, 0.995));
      REQUIRE(rating_element.ratings_[1].first == rating_type::Interchange);
      REQUIRE(equal(rating_element.ratings_[1].second, 1.0));
    }

    REQUIRE(equal(rating.connection_rating_, 0.995 * 0.995 * 0.995 * 0.9));
  };

  setup.dispatcher.on_msg(msg, 0, test_cb);
  setup.ios.run();
}
