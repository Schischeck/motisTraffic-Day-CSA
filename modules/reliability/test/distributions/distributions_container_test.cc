#include "../../include/motis/reliability/distributions/distributions_container.h"
#include "gtest/gtest.h"

namespace motis {
namespace reliability {
namespace distributions_container {

TEST(reliability_distributions_container, realtime_container) {
  container c;
  std::vector<std::pair<container::key, probability_distribution> >
      distributions;

  distributions.emplace_back(
      container::key({0, 0, "0", 0, container::key::departure, 0}),
      probability_distribution());
  distributions.back().second.init({1.0}, 0);

  distributions.emplace_back(
      container::key({0, 0, "0", 0, container::key::departure, 1}),
      probability_distribution());
  distributions.back().second.init({1.0}, 0);

  distributions.emplace_back(
      container::key({0, 0, "0", 0, container::key::arrival, 0}),
      probability_distribution());
  distributions.back().second.init({1.0}, 0);

  distributions.emplace_back(
      container::key({0, 0, "0", 1, container::key::departure, 0}),
      probability_distribution());
  distributions.back().second.init({1.0}, 0);

  distributions.emplace_back(
      container::key({0, 0, "1", 0, container::key::departure, 0}),
      probability_distribution());
  distributions.back().second.init({1.0}, 0);

  distributions.emplace_back(
      container::key({0, 1, "0", 0, container::key::departure, 0}),
      probability_distribution());
  distributions.back().second.init({1.0}, 0);

  distributions.emplace_back(
      container::key({1, 0, "0", 0, container::key::departure, 0}),
      probability_distribution());
  distributions.back().second.init({1.0}, 0);

  distributions.emplace_back(
      container::key({5, 5, "5", 5, container::key::arrival, 5}),
      probability_distribution());
  distributions.back().second.init({1.0}, 1);

  distributions.emplace_back(
      container::key({3, 10, "10", 10, container::key::departure, 10}),
      probability_distribution());
  distributions.back().second.init({1.0}, 2);

  distributions.emplace_back(
      container::key({2, 6, "1", 3, container::key::departure, 0}),
      probability_distribution());
  distributions.back().second.init({1.0}, 3);

  for (auto const& it : distributions) {
    c.add_distribution(it.first, it.second);
  }

  for (auto const& it : distributions) {
    ASSERT_EQ(it.second, c.get_distribution(it.first));
  }
}
}
}
}
