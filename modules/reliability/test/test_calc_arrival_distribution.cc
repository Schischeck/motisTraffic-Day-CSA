#include "catch/catch.hpp"

#include <iostream>

#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

#include "motis/loader/loader.h"

#include "motis/reliability/calc_arrival_distribution.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/pd_calc_data_arrival.h"
#include "motis/reliability/probability_distribution.h"
#include "motis/reliability/train_distributions.h"

#include "include/tt_distributions_test_manager.h"

using namespace motis;
using namespace motis::reliability;
using namespace motis::reliability::calc_arrival_distribution;
using namespace motis::reliability::calc_arrival_distribution::detail;

TEST_CASE("correct_rounding_errors", "[calc_arrival_distribution]") {

  std::vector<probability> probabilities = {0.1, 0.5, 0.25, 0.15};

  correct_rounding_errors(1.0, probabilities);

  REQUIRE(equal(probabilities[0], 0.1));
  REQUIRE(equal(probabilities[1], 0.5));
  REQUIRE(equal(probabilities[2], 0.25));
  REQUIRE(equal(probabilities[3], 0.15));

  probabilities[0] = 0.09999;
  correct_rounding_errors(1.0, probabilities);

  REQUIRE(equal(probabilities[0], 0.09999));
  REQUIRE(equal(probabilities[1], 0.50001));
  REQUIRE(equal(probabilities[2], 0.25));
  REQUIRE(equal(probabilities[3], 0.15));
}

TEST_CASE("compute_arrival_distribution", "[calc_arrival_distribution]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");

  train_distributions_container train_distributions(schedule->node_count);
  tt_distributions_test_manager tt_distributions;
  tt_distributions.initialize();

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node = *schedule->route_index_to_first_route_node[4];
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& light_connection = first_route_edge->_m._route_edge._conns[0];
  auto const& second_route_node = *first_route_edge->_to;

  /*pd_calc_data_arrival data(second_route_node, light_connection, *schedule,
                            train_distributions, tt_distributions);
  probability_distribution arrival_distribution;

  compute_arrival_distribution(data, arrival_distribution);*/
}
