#include "catch/catch.hpp"

#include <iostream>
#include <map>
#include <vector>

#include "motis/reliability/db_distributions_loader.h"
#include "motis/reliability/probability_distribution.h"

using namespace motis::reliability;
using namespace motis::reliability::db_distributions_loader;

TEST_CASE("load_distributions_classes", "[distributions_loader]") {
  std::map<std::string, std::string> family_to_distribution_class;
  detail::load_distributions_classes(
      "../modules/reliability/resources/distributions/Classes.csv",
      family_to_distribution_class);

  REQUIRE(family_to_distribution_class.size() == 8);
  REQUIRE(family_to_distribution_class["CNL"] == "Nachtzug");
  REQUIRE(family_to_distribution_class["EN"] == "Nachtzug");
  REQUIRE(family_to_distribution_class["HLB"].empty());
  REQUIRE(family_to_distribution_class["IC"] == "FV");
  REQUIRE(family_to_distribution_class["ICE"] == "FV");
  REQUIRE(family_to_distribution_class["RB"] == "RV");
  REQUIRE(family_to_distribution_class["RE"] == "RV");
  REQUIRE(family_to_distribution_class["S"] == "S");
}

TEST_CASE("load_distributions", "[distributions_loader]") {
  std::vector<std::pair<unsigned int, probability_distribution> > distributions;
  detail::load_distributions(
      "../modules/reliability/resources/distributions/Distributions.csv",
      distributions);

  REQUIRE(distributions.size() == 5);

  for (unsigned int id = 0; id < 5; id++) {
    REQUIRE(distributions[id].first == id);
  }
  {
    probability_distribution const& pd = distributions[0].second;
    REQUIRE(pd.first_minute() == -5);
    REQUIRE(pd.last_minute() == 10);
    REQUIRE(equal(pd.sum(), 1.0));
    REQUIRE(equal(pd.probability_equal(-5), 0.1));
    REQUIRE(equal(pd.probability_equal(-3), 0.1));
    REQUIRE(equal(pd.probability_equal(0), 0.5));
    REQUIRE(equal(pd.probability_equal(2), 0.1));
    REQUIRE(equal(pd.probability_equal(3), 0.1));
    REQUIRE(equal(pd.probability_equal(10), 0.1));
  }
  {
    probability_distribution const& pd = distributions[1].second;
    REQUIRE(pd.first_minute() == -4);
    REQUIRE(pd.last_minute() == 8);
    REQUIRE(equal(pd.sum(), 1.0));
    REQUIRE(equal(pd.probability_equal(-4), 0.1));
    REQUIRE(equal(pd.probability_equal(0), 0.8));
    REQUIRE(equal(pd.probability_equal(8), 0.1));
  }
  {
    probability_distribution const& pd = distributions[2].second;
    REQUIRE(pd.first_minute() == 0);
    REQUIRE(pd.last_minute() == 0);
    REQUIRE(equal(pd.sum(), 1.0));
    REQUIRE(equal(pd.probability_equal(0), 1.0));
  }
  {
    probability_distribution const& pd = distributions[3].second;
    REQUIRE(pd.first_minute() == -3);
    REQUIRE(pd.last_minute() == -3);
    REQUIRE(equal(pd.sum(), 1.0));
    REQUIRE(equal(pd.probability_equal(-3), 1.0));
  }
  {
    probability_distribution const& pd = distributions[4].second;
    REQUIRE(pd.first_minute() == -2);
    REQUIRE(pd.last_minute() == 2);
    REQUIRE(equal(pd.sum(), 1.0));
    REQUIRE(equal(pd.probability_equal(-2), 0.2));
    REQUIRE(equal(pd.probability_equal(-1), 0.2));
    REQUIRE(equal(pd.probability_equal(0), 0.2));
    REQUIRE(equal(pd.probability_equal(1), 0.2));
    REQUIRE(equal(pd.probability_equal(2), 0.2));
  }
}

TEST_CASE("load_mappings", "[distributions_loader]") {
  std::vector<distribution_mapping<unsigned int const> > distribution_mappings;
  detail::load_distribution_mappings(
      "../modules/reliability/resources/distributions/Mapping.csv",
      distribution_mappings);
}

TEST_CASE("load_start_distributions", "[distributions_loader]") {
  std::map<std::string, probability_distribution>
      class_to_probability_distributions;
  detail::load_start_distributions(
      "../modules/reliability/resources/distributions/StartDistributions.csv",
      class_to_probability_distributions);

  REQUIRE(class_to_probability_distributions.size() == 2);
  {
    auto const it = class_to_probability_distributions.find("FV");
    REQUIRE(it != class_to_probability_distributions.end());
    probability_distribution const& pd = it->second;
    REQUIRE(pd.first_minute() == 0);
    REQUIRE(pd.last_minute() == 3);
    REQUIRE(equal(pd.sum(), 1.0));
    REQUIRE(equal(pd.probability_equal(0), 0.8));
    REQUIRE(equal(pd.probability_equal(1), 0.1));
    REQUIRE(equal(pd.probability_equal(2), 0.0));
    REQUIRE(equal(pd.probability_equal(3), 0.1));
  }
  {
    auto const it = class_to_probability_distributions.find("RV");
    REQUIRE(it != class_to_probability_distributions.end());
    probability_distribution const& pd = it->second;
    REQUIRE(pd.first_minute() == 0);
    REQUIRE(pd.last_minute() == 1);
    REQUIRE(equal(pd.sum(), 1.0));
    REQUIRE(equal(pd.probability_equal(0), 0.9));
    REQUIRE(equal(pd.probability_equal(1), 0.1));
  }
}

TEST_CASE("mapping_is_smaller", "[distributions_loader]") {
  REQUIRE_FALSE(
      detail::mapping_is_smaller(std::make_tuple(0, "RV", 0, 5, -5, 0),
                                 std::make_tuple(1, "RV", 0, 5, -5, 0)));
  REQUIRE(detail::mapping_is_smaller(std::make_tuple(1, "FV", 0, 5, -5, 0),
                                     std::make_tuple(1, "RV", 0, 5, -5, 0)));
  REQUIRE(detail::mapping_is_smaller(std::make_tuple(1, "RV", 0, 5, -5, 0),
                                     std::make_tuple(1, "RV", 6, 10, -5, 0)));
  REQUIRE(detail::mapping_is_smaller(std::make_tuple(1, "RV", 0, 5, -5, 0),
                                     std::make_tuple(1, "RV", 0, 5, 0, 1)));
  REQUIRE(detail::mapping_is_smaller(std::make_tuple(2, "FV", 0, 5, 6, 10),
                                     std::make_tuple(1, "FV", 6, 10, 6, 10)));
}
