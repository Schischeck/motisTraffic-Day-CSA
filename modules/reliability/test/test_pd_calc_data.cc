#include "catch/catch.hpp"

#include "motis/loader/loader.h"

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/pd_calc_data_departure.h"
#include "motis/reliability/pd_calc_data_arrival.h"
#include "motis/reliability/train_distributions.h"
#include "include/tt_distributions_test_manager.h"

using namespace motis;
using namespace motis::reliability;

TEST_CASE("pd_calc_data_departure", "[pd_calc_data]") {
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
  auto const& first_light_conn = first_route_edge->_m._route_edge._conns[0];

  pd_calc_data_departure data(first_route_node, first_light_conn, true,
                              *schedule, train_distributions, tt_distributions);

  data.debug_output(std::cout);
}
