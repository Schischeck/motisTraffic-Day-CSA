#include "gtest/gtest.h"

#include "motis/reliability/distributions/probability_distribution.h"

using namespace motis;
using namespace motis::reliability;

TEST(reliability_empty_distribution, probability_distribution) {
  probability_distribution pd;

  ASSERT_TRUE(pd.empty());

  ASSERT_TRUE(pd.first_minute() == 0);
  ASSERT_TRUE(pd.last_minute() == -1);

  ASSERT_TRUE(equal(pd.probability_smaller(0), 0.0));
  ASSERT_TRUE(equal(pd.probability_smaller_equal(0), 0.0));
  ASSERT_TRUE(equal(pd.probability_equal(0), 0.0));
  ASSERT_TRUE(equal(pd.probability_greater_equal(0), 0.0));
  ASSERT_TRUE(equal(pd.probability_greater(0), 0.0));
  ASSERT_TRUE(equal(pd.sum(), 0.0));

  std::vector<probability> probabilities;
  pd.get_probabilities(probabilities);
  ASSERT_TRUE(probabilities.empty());
}

TEST(reliability_empty_distribution2, probability_distribution) {
  probability_distribution pd;
  pd.init({}, 0);
  ASSERT_TRUE(pd.empty());
}

TEST(reliability_one_point_distribution, probability_distribution) {
  probability_distribution pd;
  pd.init_one_point(0, 1.0);

  ASSERT_TRUE(pd.first_minute() == 0);
  ASSERT_TRUE(pd.last_minute() == 0);
  ASSERT_TRUE(equal(pd.sum(), 1.0));
  ASSERT_TRUE(equal(pd.probability_equal(0), 1.0));
  ASSERT_TRUE(equal(pd.probability_smaller(0), 0.0));
  ASSERT_TRUE(equal(pd.probability_greater(0), 0.0));

  pd.init_one_point(-1, 0.5);

  ASSERT_TRUE(pd.first_minute() == -1);
  ASSERT_TRUE(pd.last_minute() == -1);
  ASSERT_TRUE(equal(pd.sum(), 0.5));
  ASSERT_TRUE(equal(pd.probability_equal(-1), 0.5));
  ASSERT_TRUE(equal(pd.probability_smaller(-1), 0.0));
  ASSERT_TRUE(equal(pd.probability_greater(-1), 0.0));

  pd.init_one_point(5, 0.1);

  ASSERT_TRUE(equal(pd.first_minute(), 5));
  ASSERT_TRUE(equal(pd.last_minute(), 5));
  ASSERT_TRUE(equal(pd.sum(), 0.1));
  ASSERT_TRUE(equal(pd.probability_equal(5), 0.1));
  ASSERT_TRUE(equal(pd.probability_smaller(5), 0.0));
  ASSERT_TRUE(equal(pd.probability_greater(5), 0.0));
}

void init_test(probability_distribution const& pd, int const first_minute) {
  ASSERT_TRUE(pd.first_minute() == first_minute);
  ASSERT_TRUE(pd.last_minute() == first_minute + 2);
  ASSERT_TRUE(equal(pd.sum(), 1.0));

  ASSERT_TRUE(equal(pd.probability_smaller(first_minute - 1), 0.0));
  ASSERT_TRUE(equal(pd.probability_smaller(first_minute), 0.0));
  ASSERT_TRUE(equal(pd.probability_smaller(first_minute + 1), 0.5));
  ASSERT_TRUE(equal(pd.probability_smaller(first_minute + 2), 0.8));
  ASSERT_TRUE(equal(pd.probability_smaller(first_minute + 3), 1.0));

  ASSERT_TRUE(equal(pd.probability_smaller_equal(first_minute - 1), 0.0));
  ASSERT_TRUE(equal(pd.probability_smaller_equal(first_minute), 0.5));
  ASSERT_TRUE(equal(pd.probability_smaller_equal(first_minute + 1), 0.8));
  ASSERT_TRUE(equal(pd.probability_smaller_equal(first_minute + 2), 1.0));
  ASSERT_TRUE(equal(pd.probability_smaller_equal(first_minute + 3), 1.0));

  ASSERT_TRUE(equal(pd.probability_equal(first_minute - 1), 0.0));
  ASSERT_TRUE(equal(pd.probability_equal(first_minute), 0.5));
  ASSERT_TRUE(equal(pd.probability_equal(first_minute + 1), 0.3));
  ASSERT_TRUE(equal(pd.probability_equal(first_minute + 2), 0.2));
  ASSERT_TRUE(equal(pd.probability_equal(first_minute + 3), 0.0));

  ASSERT_TRUE(equal(pd.probability_greater_equal(first_minute - 1), 1.0));
  ASSERT_TRUE(equal(pd.probability_greater_equal(first_minute), 1.0));
  ASSERT_TRUE(equal(pd.probability_greater_equal(first_minute + 1), 0.5));
  ASSERT_TRUE(equal(pd.probability_greater_equal(first_minute + 2), 0.2));
  ASSERT_TRUE(equal(pd.probability_greater_equal(first_minute + 3), 0.0));

  ASSERT_TRUE(equal(pd.probability_greater(first_minute - 1), 1.0));
  ASSERT_TRUE(equal(pd.probability_greater(first_minute), 0.5));
  ASSERT_TRUE(equal(pd.probability_greater(first_minute + 1), 0.2));
  ASSERT_TRUE(equal(pd.probability_greater(first_minute + 2), 0.0));
  ASSERT_TRUE(equal(pd.probability_greater(first_minute + 3), 0.0));
}

TEST(reliability_init, probability_distribution) {
  std::vector<probability> const values = {0.5, 0.3, 0.2};
  for (int i = -3; i <= 1; i++) {
    probability_distribution pd;
    pd.init(values, i);
    init_test(pd, i);
  }
}

TEST(reliability_ignore_small_values, probability_distribution) {
  probability small_value = 0.0000001;
  std::vector<probability> const values = {
      small_value, small_value, 0.5 - (4 * small_value), 0.3, 0.2,
      small_value, small_value};
  for (int i = -5; i <= 1; i++) {
    probability_distribution pd;
    pd.init(values, i);
    init_test(pd, i + 2);
  }
}

TEST(reliability_only_small_values, probability_distribution) {
  probability small_value = 0.0000001;
  std::vector<probability> const values = {small_value, small_value,
                                           small_value};
  probability_distribution pd;
  pd.init(values, 0);
  ASSERT_TRUE(pd.first_minute() == 2);
  ASSERT_TRUE(pd.last_minute() == 2);
  ASSERT_TRUE(equal(pd.sum(), 3 * small_value));
}

TEST(reliability_sum_smaller_than_1, probability_distribution) {
  std::vector<probability> const values = {0.4, 0.3, 0.2};
  int const first_minute = 0;

  probability_distribution pd;
  pd.init(values, first_minute);

  ASSERT_TRUE(pd.first_minute() == first_minute);
  ASSERT_TRUE(pd.last_minute() == first_minute + 2);
  ASSERT_TRUE(equal(pd.sum(), 0.9));

  ASSERT_TRUE(equal(pd.probability_smaller(first_minute - 1), 0.0));
  ASSERT_TRUE(equal(pd.probability_smaller(first_minute), 0.0));
  ASSERT_TRUE(equal(pd.probability_smaller(first_minute + 1), 0.4));
  ASSERT_TRUE(equal(pd.probability_smaller(first_minute + 2), 0.7));
  ASSERT_TRUE(equal(pd.probability_smaller(first_minute + 3), 0.9));

  ASSERT_TRUE(equal(pd.probability_smaller_equal(first_minute - 1), 0.0));
  ASSERT_TRUE(equal(pd.probability_smaller_equal(first_minute), 0.4));
  ASSERT_TRUE(equal(pd.probability_smaller_equal(first_minute + 1), 0.7));
  ASSERT_TRUE(equal(pd.probability_smaller_equal(first_minute + 2), 0.9));
  ASSERT_TRUE(equal(pd.probability_smaller_equal(first_minute + 3), 0.9));

  ASSERT_TRUE(equal(pd.probability_equal(first_minute - 1), 0.0));
  ASSERT_TRUE(equal(pd.probability_equal(first_minute), 0.4));
  ASSERT_TRUE(equal(pd.probability_equal(first_minute + 1), 0.3));
  ASSERT_TRUE(equal(pd.probability_equal(first_minute + 2), 0.2));
  ASSERT_TRUE(equal(pd.probability_equal(first_minute + 3), 0.0));

  ASSERT_TRUE(equal(pd.probability_greater_equal(first_minute - 1), 0.9));
  ASSERT_TRUE(equal(pd.probability_greater_equal(first_minute), 0.9));
  ASSERT_TRUE(equal(pd.probability_greater_equal(first_minute + 1), 0.5));
  ASSERT_TRUE(equal(pd.probability_greater_equal(first_minute + 2), 0.2));
  ASSERT_TRUE(equal(pd.probability_greater_equal(first_minute + 3), 0.0));

  ASSERT_TRUE(equal(pd.probability_greater(first_minute - 1), 0.9));
  ASSERT_TRUE(equal(pd.probability_greater(first_minute), 0.5));
  ASSERT_TRUE(equal(pd.probability_greater(first_minute + 1), 0.2));
  ASSERT_TRUE(equal(pd.probability_greater(first_minute + 2), 0.0));
  ASSERT_TRUE(equal(pd.probability_greater(first_minute + 3), 0.0));
}

TEST(reliability_get_probabilities, probability_distribution) {
  std::vector<probability> probabilies_in = {0.5, 0.4, 0.1};
  probability_distribution pd;
  pd.init(probabilies_in, 2);

  std::vector<probability> probabilies_out;
  pd.get_probabilities(probabilies_out);

  ASSERT_TRUE(probabilies_in.size() == probabilies_out.size());
  for (unsigned int i = 0; i < probabilies_in.size(); i++)
    ASSERT_TRUE(equal(probabilies_in[i], probabilies_out[i]));
}

TEST(reliability_copy, probability_distribution) {
  probability_distribution a, b;
  a.init({0.1, 0.2, 0.5, 0.2}, -1);
  b.init(a);
  ASSERT_TRUE(b.first_minute() == a.first_minute());
  ASSERT_TRUE(b.last_minute() == a.last_minute());
  ASSERT_TRUE(equal(b.sum(), 1.0));
  ASSERT_TRUE(equal(b.probability_equal(-1), 0.1));
  ASSERT_TRUE(equal(b.probability_equal(0), 0.2));
  ASSERT_TRUE(equal(b.probability_equal(1), 0.5));
  ASSERT_TRUE(equal(b.probability_equal(2), 0.2));
}

TEST(reliability_zero, probability_distribution) {
  probability_distribution pd;
  pd.init({0.0, 0.0, 0.0}, 0);
  ASSERT_FALSE(pd.empty());
  ASSERT_TRUE(pd.first_minute() == 2);
  ASSERT_TRUE(pd.last_minute() == 2);
  ASSERT_TRUE(equal(pd.sum(), 0.0));
}

TEST(reliability_equal_operator, probability_distribution) {
  {
    probability_distribution pd1, pd2;
    pd1.init({0.5, 0.4, 0.1}, -1);
    pd2.init({0.5, 0.4, 0.1}, -1);
    ASSERT_TRUE(pd1 == pd2);
    ASSERT_FALSE(pd1 != pd2);
  }
  {
    probability_distribution pd1, pd2;
    pd1.init({0.5, 0.4, 0.1}, 0);
    pd2.init({0.5, 0.4, 0.1}, -1);
    ASSERT_FALSE(pd1 == pd2);
    ASSERT_TRUE(pd1 != pd2);
  }
  {
    probability_distribution pd1, pd2;
    pd1.init({0.5, 0.3, 0.1}, -1);
    pd2.init({0.5, 0.4, 0.1}, -1);
    ASSERT_FALSE(pd1 == pd2);
    ASSERT_TRUE(pd1 != pd2);
  }
  {
    probability_distribution pd1, pd2;
    pd1.init({0.5, 0.3, 0.1, 0.1}, -1);
    pd2.init({0.5, 0.3, 0.1}, -1);
    ASSERT_FALSE(pd1 == pd2);
    ASSERT_TRUE(pd1 != pd2);
  }
}
