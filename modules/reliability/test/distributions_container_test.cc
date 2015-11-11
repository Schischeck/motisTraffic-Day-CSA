#include "gtest/gtest.h"

#include "motis/loader/loader.h"

#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/schedule.h"

#include "motis/reliability/distributions_container.h"

using namespace motis;
using namespace motis::reliability;
using namespace motis::reliability::distributions_container;

TEST(precomputed_distributions_container, distributions_container) {
  precomputed_distributions_container container(2);

  ASSERT_FALSE(container.contains_distributions(0, arrival));
  ASSERT_FALSE(container.contains_distributions(0, departure));
  ASSERT_FALSE(container.contains_distributions(1, departure));

  container.create_route_node_distributions(0, distributions_container::arrival,
                                            2);

  ASSERT_TRUE(container.contains_distributions(0, arrival));
  ASSERT_FALSE(container.contains_distributions(0, departure));
  ASSERT_FALSE(container.contains_distributions(1, departure));

  container.create_route_node_distributions(
      0, distributions_container::departure, 1);

  ASSERT_TRUE(container.contains_distributions(0, arrival));
  ASSERT_TRUE(container.contains_distributions(0, departure));
  ASSERT_FALSE(container.contains_distributions(1, departure));

  container.create_route_node_distributions(
      1, distributions_container::departure, 1);

  ASSERT_TRUE(container.contains_distributions(0, arrival));
  ASSERT_TRUE(container.contains_distributions(0, departure));
  ASSERT_TRUE(container.contains_distributions(1, departure));

  auto& arrival_distributions00 = container.get_distribution_non_const(
      0, 0, distributions_container::arrival);
  auto& arrival_distributions01 = container.get_distribution_non_const(
      0, 1, distributions_container::arrival);
  auto& departure_distributions00 = container.get_distribution_non_const(
      0, 0, distributions_container::departure);
  auto& departure_distributions10 = container.get_distribution_non_const(
      1, 0, distributions_container::departure);

  ASSERT_TRUE(arrival_distributions00.empty());
  ASSERT_TRUE(arrival_distributions01.empty());
  ASSERT_TRUE(departure_distributions00.empty());
  ASSERT_TRUE(departure_distributions10.empty());

  arrival_distributions00.init({0.9, 0.1}, 0);
  arrival_distributions01.init({0.8, 0.2}, 0);
  departure_distributions00.init({0.7, 0.3}, 0);
  departure_distributions10.init({0.6, 0.4}, 0);

  ASSERT_TRUE(equal(arrival_distributions00.probability_equal(0), 0.9));
  ASSERT_TRUE(equal(arrival_distributions01.probability_equal(0), 0.8));
  ASSERT_TRUE(equal(departure_distributions00.probability_equal(0), 0.7));
  ASSERT_TRUE(equal(departure_distributions10.probability_equal(0), 0.6));

  ASSERT_TRUE(
      &arrival_distributions00 ==
      &container.get_distribution(0, 0, distributions_container::arrival));
  ASSERT_TRUE(
      &arrival_distributions01 ==
      &container.get_distribution(0, 1, distributions_container::arrival));
  ASSERT_TRUE(
      &departure_distributions00 ==
      &container.get_distribution(0, 0, distributions_container::departure));
  ASSERT_TRUE(
      &departure_distributions10 ==
      &container.get_distribution(1, 0, distributions_container::departure));
}

TEST(ride_distributions_container, distributions_container) {
  ride_distributions_container container;

  auto& dist1 = container.create_and_get_distribution_non_const(
      2, 1, distributions_container::departure);
  auto& dist2 = container.create_and_get_distribution_non_const(
      4, 1, distributions_container::arrival);

  dist1.init_one_point(1, 1.0);
  dist2.init_one_point(2, 1.0);

  auto const& departure_dist =
      container.get_distribution(2, 1, distributions_container::departure);
  auto const& arrival_dist =
      container.get_distribution(4, 1, distributions_container::arrival);

  ASSERT_TRUE(&dist1 == &departure_dist);
  ASSERT_TRUE(&dist2 == &arrival_dist);

  ASSERT_FALSE(departure_dist.empty());
  ASSERT_TRUE(departure_dist.first_minute() == 1);
  ASSERT_TRUE(equal(departure_dist.probability_equal(1), 1.0));

  ASSERT_FALSE(arrival_dist.empty());
  ASSERT_TRUE(arrival_dist.first_minute() == 2);
  ASSERT_TRUE(equal(arrival_dist.probability_equal(2), 1.0));
}
