#include "catch/catch.hpp"

#include "motis/reliability/probability_distribution.h"

using namespace motis;
using namespace motis::reliability;

TEST_CASE("one-point-distribution", "[probability_distribution]") {
  probability_distribution pd;
  pd.init_one_point(0, 1.0);

  REQUIRE(pd.first_minute() == 0);
  REQUIRE(pd.last_minute() == 0);
  REQUIRE(equal(pd.sum(), 1.0));
  REQUIRE(equal(pd.probability_equal(0), 1.0));
  REQUIRE(equal(pd.probability_smaller(0), 0.0));
  REQUIRE(equal(pd.probability_greater(0), 0.0));

  pd.init_one_point(-1, 0.5);

  REQUIRE(pd.first_minute() == -1);
  REQUIRE(pd.last_minute() == -1);
  REQUIRE(equal(pd.sum(), 0.5));
  REQUIRE(equal(pd.probability_equal(-1), 0.5));
  REQUIRE(equal(pd.probability_smaller(-1), 0.0));
  REQUIRE(equal(pd.probability_greater(-1), 0.0));

  pd.init_one_point(5, 0.1);

  REQUIRE(equal(pd.first_minute(), 5));
  REQUIRE(equal(pd.last_minute(), 5));
  REQUIRE(equal(pd.sum(), 0.1));
  REQUIRE(equal(pd.probability_equal(5), 0.1));
  REQUIRE(equal(pd.probability_smaller(5), 0.0));
  REQUIRE(equal(pd.probability_greater(5), 0.0));
}

void init_test(probability_distribution const& pd, int const first_minute) {
  REQUIRE(pd.first_minute() == first_minute);
  REQUIRE(pd.last_minute() == first_minute + 2);
  REQUIRE(equal(pd.sum(), 1.0));

  REQUIRE(equal(pd.probability_smaller(first_minute - 1), 0.0));
  REQUIRE(equal(pd.probability_smaller(first_minute), 0.0));
  REQUIRE(equal(pd.probability_smaller(first_minute + 1), 0.5));
  REQUIRE(equal(pd.probability_smaller(first_minute + 2), 0.8));
  REQUIRE(equal(pd.probability_smaller(first_minute + 3), 1.0));

  REQUIRE(equal(pd.probability_smaller_equal(first_minute - 1), 0.0));
  REQUIRE(equal(pd.probability_smaller_equal(first_minute), 0.5));
  REQUIRE(equal(pd.probability_smaller_equal(first_minute + 1), 0.8));
  REQUIRE(equal(pd.probability_smaller_equal(first_minute + 2), 1.0));
  REQUIRE(equal(pd.probability_smaller_equal(first_minute + 3), 1.0));

  REQUIRE(equal(pd.probability_equal(first_minute - 1), 0.0));
  REQUIRE(equal(pd.probability_equal(first_minute), 0.5));
  REQUIRE(equal(pd.probability_equal(first_minute + 1), 0.3));
  REQUIRE(equal(pd.probability_equal(first_minute + 2), 0.2));
  REQUIRE(equal(pd.probability_equal(first_minute + 3), 0.0));

  REQUIRE(equal(pd.probability_greater_equal(first_minute - 1), 1.0));
  REQUIRE(equal(pd.probability_greater_equal(first_minute), 1.0));
  REQUIRE(equal(pd.probability_greater_equal(first_minute + 1), 0.5));
  REQUIRE(equal(pd.probability_greater_equal(first_minute + 2), 0.2));
  REQUIRE(equal(pd.probability_greater_equal(first_minute + 3), 0.0));

  REQUIRE(equal(pd.probability_greater(first_minute - 1), 1.0));
  REQUIRE(equal(pd.probability_greater(first_minute), 0.5));
  REQUIRE(equal(pd.probability_greater(first_minute + 1), 0.2));
  REQUIRE(equal(pd.probability_greater(first_minute + 2), 0.0));
  REQUIRE(equal(pd.probability_greater(first_minute + 3), 0.0));
}

TEST_CASE("init", "[probability_distribution]") {
  std::vector<probability> const values = {0.5, 0.3, 0.2};
  for (int i = -3; i <= 1; i++) {
    probability_distribution pd;
    pd.init(values, i);
    init_test(pd, i);
  }
}

TEST_CASE("ignore small values", "[probability_distribution]") {
  probability small_value = 0.0000001;
  std::vector<probability> const values = {small_value, small_value,
                                           0.5 - (4 * small_value), 0.3, 0.2,
                                           small_value, small_value};
  int const first_minute = 0;
  for (int i = -5; i <= 1; i++) {
    probability_distribution pd;
    pd.init(values, i);
    init_test(pd, i + 2);
  }
}

TEST_CASE("only small values", "[probability_distribution]") {
  probability small_value = 0.0000001;
  std::vector<probability> const values = {small_value, small_value,
                                           small_value};
  probability_distribution pd;
  pd.init(values, 0);
  REQUIRE(pd.first_minute() == 2);
  REQUIRE(pd.last_minute() == 2);
  REQUIRE(equal(pd.sum(), 3 * small_value));
}

TEST_CASE("sum smaller than 1", "[probability_distribution]") {
  std::vector<probability> const values = {0.4, 0.3, 0.2};
  int const first_minute = 0;

  probability_distribution pd;
  pd.init(values, first_minute);

  REQUIRE(pd.first_minute() == first_minute);
  REQUIRE(pd.last_minute() == first_minute + 2);
  REQUIRE(equal(pd.sum(), 0.9));

  REQUIRE(equal(pd.probability_smaller(first_minute - 1), 0.0));
  REQUIRE(equal(pd.probability_smaller(first_minute), 0.0));
  REQUIRE(equal(pd.probability_smaller(first_minute + 1), 0.4));
  REQUIRE(equal(pd.probability_smaller(first_minute + 2), 0.7));
  REQUIRE(equal(pd.probability_smaller(first_minute + 3), 0.9));

  REQUIRE(equal(pd.probability_smaller_equal(first_minute - 1), 0.0));
  REQUIRE(equal(pd.probability_smaller_equal(first_minute), 0.4));
  REQUIRE(equal(pd.probability_smaller_equal(first_minute + 1), 0.7));
  REQUIRE(equal(pd.probability_smaller_equal(first_minute + 2), 0.9));
  REQUIRE(equal(pd.probability_smaller_equal(first_minute + 3), 0.9));

  REQUIRE(equal(pd.probability_equal(first_minute - 1), 0.0));
  REQUIRE(equal(pd.probability_equal(first_minute), 0.4));
  REQUIRE(equal(pd.probability_equal(first_minute + 1), 0.3));
  REQUIRE(equal(pd.probability_equal(first_minute + 2), 0.2));
  REQUIRE(equal(pd.probability_equal(first_minute + 3), 0.0));

  REQUIRE(equal(pd.probability_greater_equal(first_minute - 1), 0.9));
  REQUIRE(equal(pd.probability_greater_equal(first_minute), 0.9));
  REQUIRE(equal(pd.probability_greater_equal(first_minute + 1), 0.5));
  REQUIRE(equal(pd.probability_greater_equal(first_minute + 2), 0.2));
  REQUIRE(equal(pd.probability_greater_equal(first_minute + 3), 0.0));

  REQUIRE(equal(pd.probability_greater(first_minute - 1), 0.9));
  REQUIRE(equal(pd.probability_greater(first_minute), 0.5));
  REQUIRE(equal(pd.probability_greater(first_minute + 1), 0.2));
  REQUIRE(equal(pd.probability_greater(first_minute + 2), 0.0));
  REQUIRE(equal(pd.probability_greater(first_minute + 3), 0.0));
}

TEST_CASE("get_probabilities", "[probability_distribution]") {
  std::vector<probability> probabilies_in = {0.5, 0.4, 0.1};
  probability_distribution pd;
  pd.init(probabilies_in, 2);

  std::vector<probability> probabilies_out;
  pd.get_probabilities(probabilies_out);

  REQUIRE(probabilies_in.size() == probabilies_out.size());
  for(unsigned int i=0 ; i<probabilies_in.size() ; i++)
    REQUIRE(equal(probabilies_in[i], probabilies_out[i]));
}
