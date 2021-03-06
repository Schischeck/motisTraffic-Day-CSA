#include "motis/reliability/distributions/distributions_container.h"
#include "gtest/gtest.h"

namespace motis {
namespace reliability {
namespace distributions_container {

TEST(reliability_distributions_container, get_distribution) {
  container c;
  std::vector<std::pair<container::key, probability_distribution> >
      distributions;

  distributions.emplace_back(
      container::key({0, "ICE", "0", 0, event_type::DEP, 0}),
      probability_distribution());
  distributions.back().second.init({1.0}, 0);

  distributions.emplace_back(
      container::key({0, "ICE", "0", 0, event_type::DEP, 1}),
      probability_distribution());
  distributions.back().second.init({1.0}, 0);

  distributions.emplace_back(
      container::key({0, "ICE", "0", 0, event_type::ARR, 0}),
      probability_distribution());
  distributions.back().second.init({1.0}, 0);

  distributions.emplace_back(
      container::key({0, "ICE", "0", 1, event_type::DEP, 0}),
      probability_distribution());
  distributions.back().second.init({1.0}, 0);

  distributions.emplace_back(
      container::key({0, "ICE", "1", 0, event_type::DEP, 0}),
      probability_distribution());
  distributions.back().second.init({1.0}, 0);

  distributions.emplace_back(
      container::key({0, "RE", "0", 0, event_type::DEP, 0}),
      probability_distribution());
  distributions.back().second.init({1.0}, 0);

  distributions.emplace_back(
      container::key({1, "ICE", "0", 0, event_type::DEP, 0}),
      probability_distribution());
  distributions.back().second.init({1.0}, 0);

  distributions.emplace_back(
      container::key({5, "S", "5", 5, event_type::ARR, 5}),
      probability_distribution());
  distributions.back().second.init({1.0}, 1);

  distributions.emplace_back(
      container::key({3, "X", "10", 10, event_type::DEP, 10}),
      probability_distribution());
  distributions.back().second.init({1.0}, 2);

  distributions.emplace_back(
      container::key({2, "Y", "1", 3, event_type::DEP, 0}),
      probability_distribution());
  distributions.back().second.init({1.0}, 3);

  for (auto const& it : distributions) {
    c.get_node_non_const(it.first).pd_ = it.second;
  }

  for (auto const& it : distributions) {
    ASSERT_EQ(it.second, c.get_distribution(it.first));
  }
}
}  // namespace distributions_container
}  // namespace reliability
}  // namespace motis
