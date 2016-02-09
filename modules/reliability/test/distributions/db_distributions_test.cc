#include "gtest/gtest.h"

#include "motis/reliability/distributions/db_distributions.h"
#include "motis/reliability/distributions/probability_distribution.h"

using namespace motis::reliability;

TEST(db_distributions, reliability_start_distributions) {
  db_distributions db_dists("modules/reliability/resources/distributions/", 10,
                            10);
  {
    auto const& pd = db_dists.get_start_distribution("RB").second.get();
    ASSERT_TRUE(pd.first_minute() == 0);
    ASSERT_TRUE(pd.last_minute() == 1);
    ASSERT_TRUE(equal(pd.sum(), 1.0));
    ASSERT_TRUE(equal(pd.probability_equal(0), 0.9));
    ASSERT_TRUE(equal(pd.probability_equal(1), 0.1));
  }
  {
    auto const& pd = db_dists.get_start_distribution("ICE").second.get();
    ASSERT_TRUE(pd.first_minute() == 0);
    ASSERT_TRUE(pd.last_minute() == 3);
    ASSERT_TRUE(equal(pd.sum(), 1.0));
    ASSERT_TRUE(equal(pd.probability_equal(0), 0.8));
    ASSERT_TRUE(equal(pd.probability_equal(1), 0.1));
    ASSERT_TRUE(equal(pd.probability_equal(2), 0.0));
    ASSERT_TRUE(equal(pd.probability_equal(3), 0.1));
  }
  ASSERT_TRUE(&db_dists.get_start_distribution("RE").second.get() ==
              &db_dists.get_start_distribution("RB").second.get());
  ASSERT_TRUE(&db_dists.get_start_distribution("IC").second.get() ==
              &db_dists.get_start_distribution("ICE").second.get());
  ASSERT_FALSE(&db_dists.get_start_distribution("RE").second.get() ==
               &db_dists.get_start_distribution("ICE").second.get());
  {
    auto const& pd = db_dists.get_start_distribution("BUS").second.get();
    ASSERT_TRUE(pd.first_minute() == 0);
    ASSERT_TRUE(pd.last_minute() == 0);
    ASSERT_TRUE(equal(pd.probability_equal(0), 1.0));
  }
}

TEST(db_distributions, reliability_travel_time_distributions_Unknown) {
  db_distributions db_dists("modules/reliability/resources/distributions/", 10,
                            10);
  std::vector<start_and_travel_distributions::probability_distribution_cref>
      distributions;
  db_dists.get_travel_time_distributions("UNKNOWN", 0, 0, distributions);
  ASSERT_TRUE(distributions.empty());
}

/* RV and to_departure_delay not covered by the original mapping */
TEST(db_distributions, reliability_travel_time_distributions_RV) {
  db_distributions db_dists("modules/reliability/resources/distributions/", 10,
                            10);

  std::vector<std::string> const categories = {"RB", "RE"};
  for (auto const& category : categories) {
    for (unsigned int t = 0; t <= 10; t++) {
      std::vector<start_and_travel_distributions::probability_distribution_cref>
          distributions;
      /* 4;RV;-1;5;-2;3    note: -2 --> 0
         4;RV;5;10;-2;3 */
      // get distributions for departure delays 0 and 1
      db_dists.get_travel_time_distributions(category, t, 3, distributions);
      ASSERT_TRUE(distributions.size() == 4);
      for (unsigned int d = 0; d <= 2; d++) {
        auto const& pd = distributions[d].get();
        ASSERT_TRUE(pd.first_minute() == -2);
        ASSERT_TRUE(pd.last_minute() == 2);
        for (int d = -2; d <= 2; d++) {
          ASSERT_TRUE(equal(pd.probability_equal(d), 0.2));
        }
      }
      ASSERT_TRUE(&distributions[2].get() == &distributions[3].get());
    }
  }
}

/* FV and to_departure_delay covered by the original mapping */
TEST(db_distributions, reliability_travel_time_distributions_FV) {
  db_distributions db_dists("modules/reliability/resources/distributions/", 10,
                            10);

  std::vector<std::string> const categories = {"IC", "ICE"};
  for (auto const& category : categories) {
    for (unsigned int t = 0; t <= 5; t++) {
      std::vector<start_and_travel_distributions::probability_distribution_cref>
          distributions;
      /* 0;FV;-1;5;-5;6
       * 2;FV;-1;5;6;11 */
      db_dists.get_travel_time_distributions(category, t, 8, distributions);

      ASSERT_TRUE(distributions.size() == 9);

      for (unsigned int d = 0; d <= 5; d++) {
        auto const& pd = distributions[d].get();
        ASSERT_TRUE(pd.first_minute() == -5);
        ASSERT_TRUE(pd.last_minute() == 10);
        ASSERT_TRUE(equal(pd.probability_equal(-5), 0.1));
        ASSERT_TRUE(equal(pd.probability_equal(-3), 0.1));
        ASSERT_TRUE(equal(pd.probability_equal(0), 0.5));
        ASSERT_TRUE(equal(pd.probability_equal(2), 0.1));
        ASSERT_TRUE(equal(pd.probability_equal(3), 0.1));
        ASSERT_TRUE(equal(pd.probability_equal(10), 0.1));
      }
      for (unsigned int d = 6; d <= 8; d++) {
        auto const& pd = distributions[d].get();
        ASSERT_TRUE(pd.first_minute() == 0);
        ASSERT_TRUE(pd.last_minute() == 0);
        ASSERT_TRUE(equal(pd.probability_equal(0), 1.0));
      }
    }

    for (unsigned int t = 6; t <= 7; t++) {
      std::vector<start_and_travel_distributions::probability_distribution_cref>
          distributions;
      /* 1;FV;5;10;-5;6
       * 3;FV;5;10;6;11 */
      db_dists.get_travel_time_distributions(category, t, 7, distributions);

      ASSERT_TRUE(distributions.size() == 8);

      for (unsigned int d = 0; d <= 5; d++) {
        auto const& pd = distributions[d].get();
        ASSERT_TRUE(pd.first_minute() == -4);
        ASSERT_TRUE(pd.last_minute() == 8);
        ASSERT_TRUE(equal(pd.sum(), 1.0));
        ASSERT_TRUE(equal(pd.probability_equal(-4), 0.1));
        ASSERT_TRUE(equal(pd.probability_equal(0), 0.8));
        ASSERT_TRUE(equal(pd.probability_equal(8), 0.1));
      }
      for (unsigned int d = 6; d <= 7; d++) {
        auto const& pd = distributions[d].get();
        ASSERT_TRUE(pd.first_minute() == -3);
        ASSERT_TRUE(pd.last_minute() == -3);
        ASSERT_TRUE(equal(pd.sum(), 1.0));
        ASSERT_TRUE(equal(pd.probability_equal(-3), 1.0));
      }
    }
  }
}

/* travel time longer than the existing mappings */
TEST(db_distributions, reliability_long_travel_time) {
  db_distributions db_dists("modules/reliability/resources/distributions/", 10,
                            10);

  std::vector<start_and_travel_distributions::probability_distribution_cref>
      distributions10;
  db_dists.get_travel_time_distributions("RB", 10, 0, distributions10);

  std::vector<start_and_travel_distributions::probability_distribution_cref>
      distributions11;
  db_dists.get_travel_time_distributions("RB", 11, 0, distributions11);

  ASSERT_TRUE(distributions10.size() == distributions11.size());
  for (unsigned int i = 0; i < distributions10.size(); i++) {
    ASSERT_TRUE(&distributions10[i].get() == &distributions11[i].get());
  }
}

/* test case insensitivity */
TEST(db_distributions, case_insensitivity) {
  db_distributions db_dists("modules/reliability/resources/distributions/", 10,
                            10);

  std::vector<start_and_travel_distributions::probability_distribution_cref>
      distributions1;
  db_dists.get_travel_time_distributions("RB", 10, 0, distributions1);

  std::vector<start_and_travel_distributions::probability_distribution_cref>
      distributions2;
  db_dists.get_travel_time_distributions("rb", 11, 0, distributions2);

  ASSERT_TRUE(distributions1.size() == distributions2.size());
  for (unsigned int i = 0; i < distributions1.size(); i++) {
    ASSERT_TRUE(&distributions1[i].get() == &distributions2[i].get());
  }
}
