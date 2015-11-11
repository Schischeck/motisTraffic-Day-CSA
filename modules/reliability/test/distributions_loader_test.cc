#include "gtest/gtest.h"

#include <iostream>
#include <map>
#include <vector>

#include "motis/reliability/db_distributions_loader.h"
#include "motis/reliability/probability_distribution.h"

using namespace motis::reliability;
using namespace motis::reliability::db_distributions_loader;

TEST(load_distributions_classes, distributions_loader) {
  std::map<std::string, std::string> family_to_distribution_class;
  detail::load_distributions_classes(
      "modules/reliability/resources/distributions/Classes.csv",
      family_to_distribution_class);

  ASSERT_TRUE(family_to_distribution_class.size() == 7);
  ASSERT_TRUE(family_to_distribution_class.find("CNL")->second == "Nachtzug");
  ASSERT_TRUE(family_to_distribution_class.find("EN")->second == "Nachtzug");
  ASSERT_TRUE(family_to_distribution_class.find("HLB") ==
              family_to_distribution_class.end());
  ASSERT_TRUE(family_to_distribution_class.find("IC")->second == "FV");
  ASSERT_TRUE(family_to_distribution_class.find("ICE")->second == "FV");
  ASSERT_TRUE(family_to_distribution_class.find("RB")->second == "RV");
  ASSERT_TRUE(family_to_distribution_class.find("RE")->second == "RV");
  ASSERT_TRUE(family_to_distribution_class.find("S")->second == "S");
}

TEST(load_distributions, distributions_loader) {
  std::vector<std::pair<unsigned int, probability_distribution> > distributions;
  detail::load_distributions(
      "modules/reliability/resources/distributions/Distributions.csv",
      distributions);

  ASSERT_TRUE(distributions.size() == 6);

  for (unsigned int id = 0; id < distributions.size(); id++) {
    ASSERT_TRUE(distributions[id].first == id);
  }
  {
    probability_distribution const& pd = distributions[0].second;
    ASSERT_TRUE(pd.first_minute() == -5);
    ASSERT_TRUE(pd.last_minute() == 10);
    ASSERT_TRUE(equal(pd.sum(), 1.0));
    ASSERT_TRUE(equal(pd.probability_equal(-5), 0.1));
    ASSERT_TRUE(equal(pd.probability_equal(-3), 0.1));
    ASSERT_TRUE(equal(pd.probability_equal(0), 0.5));
    ASSERT_TRUE(equal(pd.probability_equal(2), 0.1));
    ASSERT_TRUE(equal(pd.probability_equal(3), 0.1));
    ASSERT_TRUE(equal(pd.probability_equal(10), 0.1));
  }
  {
    probability_distribution const& pd = distributions[1].second;
    ASSERT_TRUE(pd.first_minute() == -4);
    ASSERT_TRUE(pd.last_minute() == 8);
    ASSERT_TRUE(equal(pd.sum(), 1.0));
    ASSERT_TRUE(equal(pd.probability_equal(-4), 0.1));
    ASSERT_TRUE(equal(pd.probability_equal(0), 0.8));
    ASSERT_TRUE(equal(pd.probability_equal(8), 0.1));
  }
  {
    probability_distribution const& pd = distributions[2].second;
    ASSERT_TRUE(pd.first_minute() == 0);
    ASSERT_TRUE(pd.last_minute() == 0);
    ASSERT_TRUE(equal(pd.sum(), 1.0));
    ASSERT_TRUE(equal(pd.probability_equal(0), 1.0));
  }
  {
    probability_distribution const& pd = distributions[3].second;
    ASSERT_TRUE(pd.first_minute() == -3);
    ASSERT_TRUE(pd.last_minute() == -3);
    ASSERT_TRUE(equal(pd.sum(), 1.0));
    ASSERT_TRUE(equal(pd.probability_equal(-3), 1.0));
  }
  {
    probability_distribution const& pd = distributions[4].second;
    ASSERT_TRUE(pd.first_minute() == -2);
    ASSERT_TRUE(pd.last_minute() == 2);
    ASSERT_TRUE(equal(pd.sum(), 1.0));
    ASSERT_TRUE(equal(pd.probability_equal(-2), 0.2));
    ASSERT_TRUE(equal(pd.probability_equal(-1), 0.2));
    ASSERT_TRUE(equal(pd.probability_equal(0), 0.2));
    ASSERT_TRUE(equal(pd.probability_equal(1), 0.2));
    ASSERT_TRUE(equal(pd.probability_equal(2), 0.2));
  }
  {
    probability_distribution const& pd = distributions[5].second;
    ASSERT_TRUE(pd.first_minute() == -6);
    ASSERT_TRUE(pd.last_minute() == 6);
    ASSERT_TRUE(equal(pd.sum(), 1.0));
    ASSERT_TRUE(equal(pd.probability_equal(-6), 0.1));
    ASSERT_TRUE(equal(pd.probability_equal(-3), 0.1));
    ASSERT_TRUE(equal(pd.probability_equal(-1), 0.1));
    ASSERT_TRUE(equal(pd.probability_equal(1), 0.1));
    ASSERT_TRUE(equal(pd.probability_equal(3), 0.1));
    ASSERT_TRUE(equal(pd.probability_equal(6), 0.5));
  }
}

TEST(parse_travel_time_interval, distributions_loader) {

  unsigned int from_travel_time, to_travel_time;
  bool success;

  success = detail::parse_travel_time_interval("0", "2", 10, from_travel_time,
                                               to_travel_time);
  ASSERT_TRUE(success);
  ASSERT_TRUE(from_travel_time == 1);
  ASSERT_TRUE(to_travel_time == 2);

  success = detail::parse_travel_time_interval("-1", "2", 10, from_travel_time,
                                               to_travel_time);
  ASSERT_TRUE(success);
  ASSERT_TRUE(from_travel_time == 0);
  ASSERT_TRUE(to_travel_time == 2);

  success = detail::parse_travel_time_interval("-2", "2", 10, from_travel_time,
                                               to_travel_time);
  ASSERT_TRUE(success);
  ASSERT_TRUE(from_travel_time == 0);
  ASSERT_TRUE(to_travel_time == 2);

  success = detail::parse_travel_time_interval("-2", "-1", 10, from_travel_time,
                                               to_travel_time);
  ASSERT_FALSE(success);

  success = detail::parse_travel_time_interval("-2", "0", 10, from_travel_time,
                                               to_travel_time);
  ASSERT_TRUE(success);
  ASSERT_TRUE(from_travel_time == 0);
  ASSERT_TRUE(to_travel_time == 0);

  success = detail::parse_travel_time_interval("0", "10", 10, from_travel_time,
                                               to_travel_time);
  ASSERT_TRUE(success);
  ASSERT_TRUE(from_travel_time == 1);
  ASSERT_TRUE(to_travel_time == 10);

  success = detail::parse_travel_time_interval("0", "11", 10, from_travel_time,
                                               to_travel_time);
  ASSERT_TRUE(success);
  ASSERT_TRUE(from_travel_time == 1);
  ASSERT_TRUE(to_travel_time == 10);

  success = detail::parse_travel_time_interval("9", "10", 10, from_travel_time,
                                               to_travel_time);
  ASSERT_TRUE(success);
  ASSERT_TRUE(from_travel_time == 10);
  ASSERT_TRUE(to_travel_time == 10);

  success = detail::parse_travel_time_interval("10", "11", 10, from_travel_time,
                                               to_travel_time);
  ASSERT_FALSE(success);

  success = detail::parse_travel_time_interval("0", "", 10, from_travel_time,
                                               to_travel_time);
  ASSERT_TRUE(success);
  ASSERT_TRUE(from_travel_time == 1);
  ASSERT_TRUE(to_travel_time == 10);

  success = detail::parse_travel_time_interval("0", "-", 10, from_travel_time,
                                               to_travel_time);
  ASSERT_TRUE(success);
  ASSERT_TRUE(from_travel_time == 1);
  ASSERT_TRUE(to_travel_time == 10);

  success = detail::parse_travel_time_interval("10", "", 10, from_travel_time,
                                               to_travel_time);
  ASSERT_FALSE(success);
}

TEST(parse_departure_delay_interval, distributions_loader) {
  unsigned int from_delay, to_delay;
  bool success;

  success = detail::parse_departure_delay_interval("0", "2", 10, from_delay,
                                                   to_delay);
  ASSERT_TRUE(success);
  ASSERT_TRUE(from_delay == 0);
  ASSERT_TRUE(to_delay == 1);

  success = detail::parse_departure_delay_interval("-1", "2", 10, from_delay,
                                                   to_delay);
  ASSERT_TRUE(success);
  ASSERT_TRUE(from_delay == 0);
  ASSERT_TRUE(to_delay == 1);

  success = detail::parse_departure_delay_interval("-1", "0", 10, from_delay,
                                                   to_delay);
  ASSERT_FALSE(success);

  success = detail::parse_departure_delay_interval("0", "10", 10, from_delay,
                                                   to_delay);
  ASSERT_TRUE(success);
  ASSERT_TRUE(from_delay == 0);
  ASSERT_TRUE(to_delay == 9);

  success = detail::parse_departure_delay_interval("0", "11", 10, from_delay,
                                                   to_delay);
  ASSERT_TRUE(success);
  ASSERT_TRUE(from_delay == 0);
  ASSERT_TRUE(to_delay == 10);

  success = detail::parse_departure_delay_interval("0", "12", 10, from_delay,
                                                   to_delay);
  ASSERT_TRUE(success);
  ASSERT_TRUE(from_delay == 0);
  ASSERT_TRUE(to_delay == 10);

  success = detail::parse_departure_delay_interval("10", "11", 10, from_delay,
                                                   to_delay);
  ASSERT_TRUE(success);
  ASSERT_TRUE(from_delay == 10);
  ASSERT_TRUE(to_delay == 10);

  success = detail::parse_departure_delay_interval("11", "12", 10, from_delay,
                                                   to_delay);
  ASSERT_FALSE(success);

  success = detail::parse_departure_delay_interval("10", "", 10, from_delay,
                                                   to_delay);
  ASSERT_TRUE(success);
  ASSERT_TRUE(from_delay == 10);
  ASSERT_TRUE(to_delay == 10);

  success = detail::parse_departure_delay_interval("10", "-", 10, from_delay,
                                                   to_delay);
  ASSERT_TRUE(success);
  ASSERT_TRUE(from_delay == 10);
  ASSERT_TRUE(to_delay == 10);

  success = detail::parse_departure_delay_interval("11", "", 10, from_delay,
                                                   to_delay);
  ASSERT_FALSE(success);
}

TEST(to_resolved_mappings, distributions_loader) {
  std::vector<detail::mapping_int> integer_mappings;

  integer_mappings.emplace_back(1, "RV", 0, 2, 0, 1);
  integer_mappings.emplace_back(2, "RV", 0, 2, 2, 3);
  integer_mappings.emplace_back(3, "RV", 3, 5, 0, 2);
  integer_mappings.emplace_back(4, "RV", 0, 10, 4, 10);

  std::vector<resolved_mapping> resolved_mappings;
  detail::resolve_mappings(integer_mappings, resolved_mappings);

  for (auto const& mapping : resolved_mappings) {
    ASSERT_TRUE(std::get<resolved_mapping_pos::rm_class>(mapping) == "RV");
  }

  unsigned int mapping_index = 0;
  for (unsigned int t = 0; t <= 2; t++) {
    for (unsigned int d = 0; d <= 1; d++) {
      auto const& mapping = resolved_mappings[mapping_index++];
      ASSERT_TRUE(std::get<resolved_mapping_pos::rm_distribution_id>(mapping) ==
                  1);
      ASSERT_TRUE(std::get<resolved_mapping_pos::rm_travel_time>(mapping) == t);
      ASSERT_TRUE(std::get<resolved_mapping_pos::rm_delay>(mapping) == d);
    }
  }
  for (unsigned int t = 0; t <= 2; t++) {
    for (unsigned int d = 2; d <= 3; d++) {
      auto const& mapping = resolved_mappings[mapping_index++];
      ASSERT_TRUE(std::get<resolved_mapping_pos::rm_distribution_id>(mapping) ==
                  2);
      ASSERT_TRUE(std::get<resolved_mapping_pos::rm_travel_time>(mapping) == t);
      ASSERT_TRUE(std::get<resolved_mapping_pos::rm_delay>(mapping) == d);
    }
  }
  for (unsigned int t = 3; t <= 5; t++) {
    for (unsigned int d = 0; d <= 2; d++) {
      auto const& mapping = resolved_mappings[mapping_index++];
      ASSERT_TRUE(std::get<resolved_mapping_pos::rm_distribution_id>(mapping) ==
                  3);
      ASSERT_TRUE(std::get<resolved_mapping_pos::rm_travel_time>(mapping) == t);
      ASSERT_TRUE(std::get<resolved_mapping_pos::rm_delay>(mapping) == d);
    }
  }
  for (unsigned int t = 0; t <= 10; t++) {
    for (unsigned int d = 4; d <= 10; d++) {
      auto const& mapping = resolved_mappings[mapping_index++];
      ASSERT_TRUE(std::get<resolved_mapping_pos::rm_distribution_id>(mapping) ==
                  4);
      ASSERT_TRUE(std::get<resolved_mapping_pos::rm_travel_time>(mapping) == t);
      ASSERT_TRUE(std::get<resolved_mapping_pos::rm_delay>(mapping) == d);
    }
  }

  ASSERT_TRUE(resolved_mappings.size() == mapping_index);
}
#include <cassert>
void test_mapping(std::vector<resolved_mapping> const& distribution_mappings,
                  unsigned int& distribution_mappings_idx,
                  unsigned int const distribution_id,
                  std::string const& distribution_class,
                  unsigned int const travel_time_from,
                  unsigned int const travel_time_to,
                  unsigned int const delay_from, unsigned int const delay_to) {
  for (unsigned int t = travel_time_from; t <= travel_time_to; t++) {
    for (unsigned int d = delay_from; d <= delay_to; d++) {
      auto const& mapping = distribution_mappings[distribution_mappings_idx++];
      ASSERT_TRUE(std::get<resolved_mapping_pos::rm_distribution_id>(mapping) ==
                  distribution_id);
      ASSERT_TRUE(std::get<resolved_mapping_pos::rm_class>(mapping) ==
                  distribution_class);
      ASSERT_TRUE(std::get<resolved_mapping_pos::rm_travel_time>(mapping) == t);
      ASSERT_TRUE(std::get<resolved_mapping_pos::rm_delay>(mapping) == d);
    }
  }
}

TEST(load_mappings, distributions_loader) {
  std::vector<resolved_mapping> distribution_mappings;
  detail::load_distribution_mappings(
      "modules/reliability/resources/distributions/Mapping.csv", 10, 10,
      distribution_mappings);

  unsigned int distribution_mappings_idx = 0;

  for (unsigned int t = 0; t <= 5; t++) {
    // 0;FV;-1;5;-5;6
    test_mapping(distribution_mappings, distribution_mappings_idx, 0, "FV", t,
                 t, 0, 5);
    // 2;FV;-1;5;6;11
    test_mapping(distribution_mappings, distribution_mappings_idx, 2, "FV", t,
                 t, 6, 10);
  }

  for (unsigned int t = 6; t <= 10; t++) {
    // 1;FV;5;10;-5;6
    test_mapping(distribution_mappings, distribution_mappings_idx, 1, "FV", t,
                 t, 0, 5);
    // 3;FV;5;10;6;11
    test_mapping(distribution_mappings, distribution_mappings_idx, 3, "FV", t,
                 t, 6, 10);
  }

  for (unsigned int t = 0; t <= 5; t++) {
    // 4;RV;-1;5;-2;3
    test_mapping(distribution_mappings, distribution_mappings_idx, 4, "RV", t,
                 t, 0, 2);
  }

  for (unsigned int t = 6; t <= 10; t++) {
    // 4;RV;5;10;-2;3
    test_mapping(distribution_mappings, distribution_mappings_idx, 4, "RV", t,
                 t, 0, 2);
  }

  ASSERT_TRUE(distribution_mappings.size() == distribution_mappings_idx);
}

/* negative values and unlimited intervals */
#include <climits>
TEST(load_mappings2, distributions_loader) {
  std::vector<resolved_mapping> distribution_mappings;
  detail::load_distribution_mappings(
      "modules/reliability/resources/distributions/Mapping2.csv", 10, 10,
      distribution_mappings);

  unsigned int distribution_mappings_idx = 0;

  for (unsigned int t = 0; t <= 1; t++) {
    // 6;FV;-3;1;-4;2
    test_mapping(distribution_mappings, distribution_mappings_idx, 6, "FV", t,
                 t, 0, 1);
    // 4;FV;-3;1;2;4
    test_mapping(distribution_mappings, distribution_mappings_idx, 4, "FV", t,
                 t, 2, 3);
  }

  // 5;FV;1;3;-2;2
  for (unsigned int t = 2; t <= 3; t++) {
    test_mapping(distribution_mappings, distribution_mappings_idx, 5, "FV", t,
                 t, 0, 1);
  }

  // 7;FV;3;5;4;6
  for (unsigned int t = 4; t <= 5; t++) {
    test_mapping(distribution_mappings, distribution_mappings_idx, 7, "FV", t,
                 t, 4, 5);
  }

  // 8;FV;5;-;0;
  test_mapping(distribution_mappings, distribution_mappings_idx, 8, "FV", 6, 10,
               0, 10);

  // 9;RV;5;-;0;-
  test_mapping(distribution_mappings, distribution_mappings_idx, 9, "RV", 6, 10,
               0, 10);

  ASSERT_TRUE(distribution_mappings.size() == distribution_mappings_idx);
}

/* overlapping intervals */
TEST(load_mappings3, distributions_loader) {
  std::vector<resolved_mapping> distribution_mappings;
  detail::load_distribution_mappings(
      "modules/reliability/resources/distributions/Mapping3.csv", 10, 10,
      distribution_mappings);

  unsigned int distribution_mappings_idx = 0;
  for (unsigned int t = 0; t <= 10; t++) {
    if (t <= 5) {
      test_mapping(distribution_mappings, distribution_mappings_idx, 3, "FV", t,
                   t, 0, 2);
      test_mapping(distribution_mappings, distribution_mappings_idx, 1, "FV", t,
                   t, 3, 4);
    }
    test_mapping(distribution_mappings, distribution_mappings_idx, 2, "FV", t,
                 t, 5, 10);
  }

  ASSERT_TRUE(distribution_mappings.size() == distribution_mappings_idx);
}

TEST(load_start_distributions, distributions_loader) {
  std::map<std::string, probability_distribution>
      class_to_probability_distributions;
  detail::load_start_distributions(
      "modules/reliability/resources/distributions/StartDistributions.csv",
      class_to_probability_distributions);

  ASSERT_TRUE(class_to_probability_distributions.size() == 2);
  {
    auto const it = class_to_probability_distributions.find("FV");
    ASSERT_TRUE(it != class_to_probability_distributions.end());
    probability_distribution const& pd = it->second;
    ASSERT_TRUE(pd.first_minute() == 0);
    ASSERT_TRUE(pd.last_minute() == 3);
    ASSERT_TRUE(equal(pd.sum(), 1.0));
    ASSERT_TRUE(equal(pd.probability_equal(0), 0.8));
    ASSERT_TRUE(equal(pd.probability_equal(1), 0.1));
    ASSERT_TRUE(equal(pd.probability_equal(2), 0.0));
    ASSERT_TRUE(equal(pd.probability_equal(3), 0.1));
  }
  {
    auto const it = class_to_probability_distributions.find("RV");
    ASSERT_TRUE(it != class_to_probability_distributions.end());
    probability_distribution const& pd = it->second;
    ASSERT_TRUE(pd.first_minute() == 0);
    ASSERT_TRUE(pd.last_minute() == 1);
    ASSERT_TRUE(equal(pd.sum(), 1.0));
    ASSERT_TRUE(equal(pd.probability_equal(0), 0.9));
    ASSERT_TRUE(equal(pd.probability_equal(1), 0.1));
  }
}

TEST(mapping_is_smaller, distributions_loader) {
  ASSERT_TRUE(detail::mapping_is_smaller(std::make_tuple("FV", 1, 1, 1),
                                         std::make_tuple("RV", 0, 0, 0)));
  ASSERT_FALSE(detail::mapping_is_smaller(std::make_tuple("RV", 0, 0, 0),
                                          std::make_tuple("FV", 1, 1, 1)));

  ASSERT_TRUE(detail::mapping_is_smaller(std::make_tuple("FV", 0, 1, 1),
                                         std::make_tuple("FV", 1, 0, 0)));
  ASSERT_FALSE(detail::mapping_is_smaller(std::make_tuple("FV", 1, 0, 0),
                                          std::make_tuple("FV", 0, 1, 1)));

  ASSERT_TRUE(detail::mapping_is_smaller(std::make_tuple("FV", 0, 0, 1),
                                         std::make_tuple("FV", 0, 1, 0)));
  ASSERT_FALSE(detail::mapping_is_smaller(std::make_tuple("FV", 0, 1, 0),
                                          std::make_tuple("FV", 0, 0, 1)));

  ASSERT_FALSE(detail::mapping_is_smaller(std::make_tuple("FV", 0, 0, 0),
                                          std::make_tuple("FV", 0, 0, 1)));
  ASSERT_FALSE(detail::mapping_is_smaller(std::make_tuple("FV", 0, 0, 1),
                                          std::make_tuple("FV", 0, 0, 0)));
}
