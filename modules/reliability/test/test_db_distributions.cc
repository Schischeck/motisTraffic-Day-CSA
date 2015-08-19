#include "catch/catch.hpp"

#include "motis/reliability/db_distributions.h"
#include "motis/reliability/probability_distribution.h"

using namespace motis::reliability;

TEST_CASE("start_distributions", "[db_distributions]") {
  db_distributions db_dists("../modules/reliability/resources/distributions/");
  {
    auto const& pd = db_dists.get_start_distribution("RB");
    REQUIRE(pd.first_minute() == 0);
    REQUIRE(pd.last_minute() == 1);
    REQUIRE(equal(pd.sum(), 1.0));
    REQUIRE(equal(pd.probability_equal(0), 0.9));
    REQUIRE(equal(pd.probability_equal(1), 0.1));
  }
  {
    auto const& pd = db_dists.get_start_distribution("ICE");
    REQUIRE(pd.first_minute() == 0);
    REQUIRE(pd.last_minute() == 3);
    REQUIRE(equal(pd.sum(), 1.0));
    REQUIRE(equal(pd.probability_equal(0), 0.8));
    REQUIRE(equal(pd.probability_equal(1), 0.1));
    REQUIRE(equal(pd.probability_equal(2), 0.0));
    REQUIRE(equal(pd.probability_equal(3), 0.1));
  }
  REQUIRE(&db_dists.get_start_distribution("RE") ==
          &db_dists.get_start_distribution("RB"));
  REQUIRE(&db_dists.get_start_distribution("IC") ==
          &db_dists.get_start_distribution("ICE"));
  REQUIRE_FALSE(&db_dists.get_start_distribution("RE") ==
                &db_dists.get_start_distribution("ICE"));
  {
    auto const& pd = db_dists.get_start_distribution("BUS");
    REQUIRE(pd.first_minute() == 0);
    REQUIRE(pd.last_minute() == 0);
    REQUIRE(equal(pd.probability_equal(0), 1.0));
  }
}

TEST_CASE("travel_time_distributions_RV", "[db_distributions]") {
  db_distributions db_dists("../modules/reliability/resources/distributions/");

  std::vector<std::string> const categories = {"RB", "RE"};
  for (auto const& category : categories) {
    for (unsigned int t = 0; t <= 10; t++) {
      std::vector<db_distributions::travel_time_distribution> distributions;
      /* 4;RV;0;5;-2;2
         4;RV;6;10;-2;2 */
      db_dists.get_travel_time_distributions(category, t, distributions);
      REQUIRE(distributions.size() == 1);
      REQUIRE(distributions[0].departure_delay_from_ == -2);
      REQUIRE(distributions[0].departure_delay_to_ == 2);
      auto const& pd = distributions[0].distribution_;
      REQUIRE(pd.first_minute() == -2);
      REQUIRE(pd.last_minute() == 2);
      for (int d = -2; d <= 2; d++) {
        REQUIRE(equal(pd.probability_equal(d), 0.2));
      }
    }

    std::vector<db_distributions::travel_time_distribution> distributions;
    db_dists.get_travel_time_distributions(category, -1, distributions);
    REQUIRE(distributions.empty());
    db_dists.get_travel_time_distributions(category, 11, distributions);
    REQUIRE(distributions.empty());
  }
}

TEST_CASE("travel_time_distributions_FV", "[db_distributions]") {
  db_distributions db_dists("../modules/reliability/resources/distributions/");

  std::vector<std::string> const categories = {"IC", "ICE"};
  for (auto const& category : categories) {
    for (unsigned int t = 0; t <= 5; t++) {
      std::vector<db_distributions::travel_time_distribution> distributions;
      /* 0;FV;0;5;-5;5
       * 2;FV;0;5;6;10 */
      db_dists.get_travel_time_distributions(category, t, distributions);
      REQUIRE(distributions.size() == 2);
      REQUIRE(distributions[0].departure_delay_from_ == -5);
      REQUIRE(distributions[0].departure_delay_to_ == 5);
      REQUIRE(distributions[1].departure_delay_from_ == 6);
      REQUIRE(distributions[1].departure_delay_to_ == 10);
      {
        auto const& pd = distributions[0].distribution_;
        REQUIRE(pd.first_minute() == -5);
        REQUIRE(pd.last_minute() == 10);
        REQUIRE(equal(pd.probability_equal(-5), 0.1));
        REQUIRE(equal(pd.probability_equal(-3), 0.1));
        REQUIRE(equal(pd.probability_equal(0), 0.5));
        REQUIRE(equal(pd.probability_equal(2), 0.1));
        REQUIRE(equal(pd.probability_equal(3), 0.1));
        REQUIRE(equal(pd.probability_equal(10), 0.1));
      }
      {
        auto const& pd = distributions[1].distribution_;
        REQUIRE(pd.first_minute() == 0);
        REQUIRE(pd.last_minute() == 0);
        REQUIRE(equal(pd.probability_equal(0), 1.0));
      }
    }

    for (unsigned int t = 6; t <= 10; t++) {
      std::vector<db_distributions::travel_time_distribution> distributions;
      /* 1;FV;6;10;-5;5
       * 3;FV;6;10;6;10 */
      db_dists.get_travel_time_distributions(category, t, distributions);
      REQUIRE(distributions.size() == 2);
      REQUIRE(distributions[0].departure_delay_from_ == -5);
      REQUIRE(distributions[0].departure_delay_to_ == 5);
      REQUIRE(distributions[1].departure_delay_from_ == 6);
      REQUIRE(distributions[1].departure_delay_to_ == 10);
      {
        auto const& pd = distributions[0].distribution_;
        REQUIRE(pd.first_minute() == -4);
        REQUIRE(pd.last_minute() == 8);
        REQUIRE(equal(pd.sum(), 1.0));
        REQUIRE(equal(pd.probability_equal(-4), 0.1));
        REQUIRE(equal(pd.probability_equal(0), 0.8));
        REQUIRE(equal(pd.probability_equal(8), 0.1));
      }
      {
        auto const& pd = distributions[1].distribution_;
        REQUIRE(pd.first_minute() == -3);
        REQUIRE(pd.last_minute() == -3);
        REQUIRE(equal(pd.sum(), 1.0));
        REQUIRE(equal(pd.probability_equal(-3), 1.0));
      }
    }

    std::vector<db_distributions::travel_time_distribution> distributions;
    db_dists.get_travel_time_distributions(category, -1, distributions);
    REQUIRE(distributions.empty());
    db_dists.get_travel_time_distributions(category, 11, distributions);
    REQUIRE(distributions.empty());
  }
}
