#include "catch/catch.hpp"

#include "motis/loader/loader.h"

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/pd_calc_data_arrival.h"
#include "motis/reliability/train_distributions.h"
#include "include/tt_distributions_test_manager.h"

using namespace motis;
using namespace motis::reliability;

struct train_distributions_test_container : train_distributions_container {
  train_distributions_test_container(std::vector<probability> probabilities,
                                     unsigned num_nodes)
      : train_distributions_container(num_nodes) {
    dist.init(probabilities, 0);
  }

  probability_distribution const& get_train_distribution(
      unsigned int const route_node_idx, unsigned int const light_conn_idx,
      type const t) const {
    (void)route_node_idx;
    (void)light_conn_idx;
    (void)t;
    return dist;
  }

  probability_distribution dist;
};

TEST_CASE("initialize", "[pd_calc_data_arrival]") {
  auto schedule =
      load_text_schedule("../modules/reliability/resources/schedule/motis");

  train_distributions_test_container train_distributions({0.8, 0.2}, schedule->node_count);
  tt_distributions_test_manager tt_distributions;
  tt_distributions.initialize();

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node = *schedule->route_index_to_first_route_node[4];
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge =
      graph_accessor::get_departing_route_edge(first_route_node);
  auto const& light_connection = first_route_edge->_m._route_edge._conns[0];
  auto const& second_route_node = *first_route_edge->_to;

  pd_calc_data_arrival data(second_route_node, light_connection, *schedule,
                            train_distributions, tt_distributions);
}
