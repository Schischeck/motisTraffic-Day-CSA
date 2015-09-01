#include "catch/catch.hpp"

#include "motis/loader/loader.h"

#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/schedule.h"

#include "motis/reliability/distributions_container.h"

using namespace motis;
using namespace motis::reliability;

TEST_CASE("test", "[train_dist_container]") {
  train_distributions_container container(2);

  REQUIRE_FALSE(container.contains_arrival_distributions(0));
  REQUIRE_FALSE(container.contains_departure_distributions(0));
  REQUIRE_FALSE(container.contains_departure_distributions(1));

  container.create_train_distributions(
      0, train_distributions_container::arrival, 2);

  REQUIRE(container.contains_arrival_distributions(0));
  REQUIRE_FALSE(container.contains_departure_distributions(0));
  REQUIRE_FALSE(container.contains_departure_distributions(1));

  container.create_train_distributions(
      0, train_distributions_container::departure, 1);

  REQUIRE(container.contains_arrival_distributions(0));
  REQUIRE(container.contains_departure_distributions(0));
  REQUIRE_FALSE(container.contains_departure_distributions(1));

  container.create_train_distributions(
      1, train_distributions_container::departure, 1);

  REQUIRE(container.contains_arrival_distributions(0));
  REQUIRE(container.contains_departure_distributions(0));
  REQUIRE(container.contains_departure_distributions(1));

  auto& arrival_distributions = container.get_train_distributions(
      0, train_distributions_container::arrival);
  auto& departure_distributions = container.get_train_distributions(
      0, train_distributions_container::departure);
  auto& departure_distributions2 = container.get_train_distributions(
      1, train_distributions_container::departure);

  REQUIRE(arrival_distributions.get_distribution(0).empty());
  REQUIRE(arrival_distributions.get_distribution(1).empty());
  REQUIRE(departure_distributions.get_distribution(0).empty());
  REQUIRE(departure_distributions2.get_distribution(0).empty());

  arrival_distributions.get_distribution_non_const(0).init({0.9, 0.1}, 0);
  arrival_distributions.get_distribution_non_const(1).init({0.8, 0.2}, 0);
  departure_distributions.get_distribution_non_const(0).init({0.7, 0.3}, 0);
  departure_distributions2.get_distribution_non_const(0).init({0.6, 0.4}, 0);

  REQUIRE(equal(arrival_distributions.get_distribution(0).probability_equal(0),
                0.9));
  REQUIRE(equal(arrival_distributions.get_distribution(1).probability_equal(0),
                0.8));
  REQUIRE(equal(
      departure_distributions.get_distribution(0).probability_equal(0), 0.7));
  REQUIRE(equal(
      departure_distributions2.get_distribution(0).probability_equal(0), 0.6));

  REQUIRE(&arrival_distributions.get_distribution(0) ==
          &container.get_probability_distribution(
              0, 0, train_distributions_container::arrival));
  REQUIRE(&arrival_distributions.get_distribution(1) ==
          &container.get_probability_distribution(
              0, 1, train_distributions_container::arrival));
  REQUIRE(&departure_distributions.get_distribution(0) ==
          &container.get_probability_distribution(
              0, 0, train_distributions_container::departure));
  REQUIRE(&departure_distributions2.get_distribution(0) ==
          &container.get_probability_distribution(
              1, 0, train_distributions_container::departure));
}
