#include "gtest/gtest.h"

#include "motis/reliability/distributions/probability_distribution.h"
#include "motis/reliability/distributions/s_t_distributions_container.h"

using namespace motis::reliability;

std::vector<s_t_distributions_container::parameters> get_parameters() {
  std::vector<s_t_distributions_container::parameters> param;
  param.emplace_back();
  param.back().root_ = "modules/reliability/resources/distributions/";
  param.back().max_expected_travel_time_ = 10;
  param.back().max_expected_departure_delay_ = 10;
  param.emplace_back();
  param.back().root_ = "modules/reliability/resources/distributions2/";
  param.back().max_expected_travel_time_ = 10;
  param.back().max_expected_departure_delay_ = 10;
  return param;
}

TEST(s_t_distributions_container, start_distributions) {
  s_t_distributions_container distributions(get_parameters());
  {
    auto const start_dist = distributions.get_start_distribution("dummy");
    ASSERT_FALSE(start_dist.first);
    auto const& pd = start_dist.second.get();
    ASSERT_EQ(0, pd.first_minute());
    ASSERT_EQ(0, pd.last_minute());
    ASSERT_TRUE(equal(1.0, pd.probability_equal(0)));
  }
  {
    auto const start_dist = distributions.get_start_distribution("bus");
    ASSERT_TRUE(start_dist.first);
    auto const& pd = start_dist.second.get();
    ASSERT_EQ(0, pd.first_minute());
    ASSERT_EQ(1, pd.last_minute());
    ASSERT_TRUE(equal(0.62, pd.probability_equal(0)));
    ASSERT_TRUE(equal(0.38, pd.probability_equal(1)));
  }
  {
    auto const start_dist = distributions.get_start_distribution("RB");
    ASSERT_TRUE(start_dist.first);
    auto const& pd = start_dist.second.get();
    ASSERT_TRUE(pd.first_minute() == 0);
    ASSERT_TRUE(pd.last_minute() == 1);
    ASSERT_TRUE(equal(pd.sum(), 1.0));
    ASSERT_TRUE(equal(pd.probability_equal(0), 0.9));
    ASSERT_TRUE(equal(pd.probability_equal(1), 0.1));
  }
}

TEST(s_t_distributions_container, travel_time_distributions) {
  s_t_distributions_container distributions(get_parameters());
  {
    std::vector<start_and_travel_distributions::probability_distribution_cref>
        tt_dists;
    auto const success =
        distributions.get_travel_time_distributions("dummy", 1, 0, tt_dists);
    ASSERT_FALSE(success);
    ASSERT_TRUE(tt_dists.empty());
  }
  {
    std::vector<start_and_travel_distributions::probability_distribution_cref>
        tt_dists;
    auto const success =
        distributions.get_travel_time_distributions("bus", 1, 1, tt_dists);
    ASSERT_TRUE(success);
    ASSERT_EQ(2, tt_dists.size());
    for (auto const& pd : tt_dists) {
      ASSERT_EQ(0, pd.get().first_minute());
      ASSERT_EQ(2, pd.get().last_minute());
      ASSERT_TRUE(equal(0.5, pd.get().probability_equal(0)));
      ASSERT_TRUE(equal(0.4, pd.get().probability_equal(1)));
      ASSERT_TRUE(equal(0.1, pd.get().probability_equal(2)));
    }
  }
  {
    std::vector<start_and_travel_distributions::probability_distribution_cref>
        tt_dists;
    auto const success =
        distributions.get_travel_time_distributions("ice", 6, 7, tt_dists);
    ASSERT_TRUE(success);
    ASSERT_EQ(8, tt_dists.size());
    auto const& pd = tt_dists[7].get();
    ASSERT_EQ(-3, pd.first_minute());
    ASSERT_EQ(-3, pd.last_minute());
    ASSERT_TRUE(equal(1.0, pd.probability_equal(-3)));
  }
}
