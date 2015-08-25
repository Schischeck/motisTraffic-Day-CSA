#include "catch/catch.hpp"

#include "motis/reliability/db_distributions.h"
#include "motis/reliability/probability_distribution.h"

using namespace motis::reliability;

TEST_CASE("start_distributions", "[db_distributions]") {
  db_distributions db_dists("../modules/reliability/resources/distributions/",
                            10, 10);
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

TEST_CASE("travel_time_distributions_Unknown", "[db_distributions]") {
  db_distributions db_dists("../modules/reliability/resources/distributions/",
                            10, 10);
  std::vector<start_and_travel_distributions::probability_distribution_cref>
      distributions;
  db_dists.get_travel_time_distributions("UNKNOWN", 0, 0, distributions);
  REQUIRE(distributions.empty());
}

/* RV and to_departure_delay not covered by the original mapping */
TEST_CASE("travel_time_distributions_RV", "[db_distributions]") {
  db_distributions db_dists("../modules/reliability/resources/distributions/",
                            10, 10);

  std::vector<std::string> const categories = {"RB", "RE"};
  for (auto const& category : categories) {
    for (unsigned int t = 0; t <= 10; t++) {
      std::vector<start_and_travel_distributions::probability_distribution_cref>
          distributions;
      /* 4;RV;-1;5;-2;3    note: -2 --> 0
         4;RV;5;10;-2;3 */
      // get distributions for departure delays 0 and 1
      db_dists.get_travel_time_distributions(category, t, 3, distributions);
      REQUIRE(distributions.size() == 4);
      for (unsigned int d = 0; d <= 2; d++) {
        auto const& pd = distributions[d].get();
        REQUIRE(pd.first_minute() == -2);
        REQUIRE(pd.last_minute() == 2);
        for (int d = -2; d <= 2; d++) {
          REQUIRE(equal(pd.probability_equal(d), 0.2));
        }
      }
      REQUIRE(&distributions[2].get() == &distributions[3].get());
    }
  }
}

/* FV and to_departure_delay covered by the original mapping */
TEST_CASE("travel_time_distributions_FV", "[db_distributions]") {
  db_distributions db_dists("../modules/reliability/resources/distributions/",
                            10, 10);

  std::vector<std::string> const categories = {"IC", "ICE"};
  for (auto const& category : categories) {
    for (unsigned int t = 0; t <= 5; t++) {
      std::vector<start_and_travel_distributions::probability_distribution_cref>
          distributions;
      /* 0;FV;-1;5;-5;6
       * 2;FV;-1;5;6;11 */
      db_dists.get_travel_time_distributions(category, t, 8, distributions);

      REQUIRE(distributions.size() == 9);

      for (unsigned int d = 0; d <= 5; d++) {
        auto const& pd = distributions[d].get();
        REQUIRE(pd.first_minute() == -5);
        REQUIRE(pd.last_minute() == 10);
        REQUIRE(equal(pd.probability_equal(-5), 0.1));
        REQUIRE(equal(pd.probability_equal(-3), 0.1));
        REQUIRE(equal(pd.probability_equal(0), 0.5));
        REQUIRE(equal(pd.probability_equal(2), 0.1));
        REQUIRE(equal(pd.probability_equal(3), 0.1));
        REQUIRE(equal(pd.probability_equal(10), 0.1));
      }
      for (unsigned int d = 6; d <= 8; d++) {
        auto const& pd = distributions[d].get();
        REQUIRE(pd.first_minute() == 0);
        REQUIRE(pd.last_minute() == 0);
        REQUIRE(equal(pd.probability_equal(0), 1.0));
      }
    }

    for (unsigned int t = 6; t <= 7; t++) {
      std::vector<start_and_travel_distributions::probability_distribution_cref>
          distributions;
      /* 1;FV;5;10;-5;6
       * 3;FV;5;10;6;11 */
      db_dists.get_travel_time_distributions(category, t, 7, distributions);

      REQUIRE(distributions.size() == 8);

      for (unsigned int d = 0; d <= 5; d++) {
        auto const& pd = distributions[d].get();
        REQUIRE(pd.first_minute() == -4);
        REQUIRE(pd.last_minute() == 8);
        REQUIRE(equal(pd.sum(), 1.0));
        REQUIRE(equal(pd.probability_equal(-4), 0.1));
        REQUIRE(equal(pd.probability_equal(0), 0.8));
        REQUIRE(equal(pd.probability_equal(8), 0.1));
      }
      for (unsigned int d = 6; d <= 7; d++) {
        auto const& pd = distributions[d].get();
        REQUIRE(pd.first_minute() == -3);
        REQUIRE(pd.last_minute() == -3);
        REQUIRE(equal(pd.sum(), 1.0));
        REQUIRE(equal(pd.probability_equal(-3), 1.0));
      }
    }
  }
}

/* travel time longer than the existing mappings */
TEST_CASE("long_travel_time", "[db_distributions]") {
  db_distributions db_dists("../modules/reliability/resources/distributions/",
                            10, 10);

  std::vector<start_and_travel_distributions::probability_distribution_cref>
      distributions10;
  db_dists.get_travel_time_distributions("RB", 10, 0, distributions10);

  std::vector<start_and_travel_distributions::probability_distribution_cref>
      distributions11;
  db_dists.get_travel_time_distributions("RB", 11, 0, distributions11);

  REQUIRE(distributions10.size() == distributions11.size());
  for (unsigned int i = 0; i < distributions10.size(); i++) {
    REQUIRE(&distributions10[i].get() == &distributions11[i].get());
  }
}
