#include "gtest/gtest.h"

#include "motis/loader/loader.h"

#include "motis/core/common/date_util.h"

#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/schedule.h"

#include "motis/reliability/computation/calc_departure_distribution.h"
#include "motis/reliability/computation/data_departure.h"
#include "motis/reliability/computation/distributions_calculator.h"
#include "motis/reliability/computation/ride_distributions_calculator.h"
#include "motis/reliability/context.h"
#include "motis/reliability/distributions/distributions_container.h"
#include "motis/reliability/graph_accessor.h"

#include "../include/start_and_travel_test_distributions.h"
#include "../include/test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace distributions_calculator {

class reliability_distributions_calculator : public test_schedule_setup {
public:
  reliability_distributions_calculator()
      : test_schedule_setup("modules/reliability/resources/schedule/",
                            "20150928") {}
  /* train numbers */
  short const IC_DA_H = 1;
  short const IC_FR_DA = 2;
  short const IC_FH_DA = 3;
  short const RE_MA_DA = 4;
  short const ICE_FR_DA_H = 5;
  short const ICE_HA_W_HE = 6;
  short const ICE_K_K = 7;
  short const RE_K_S = 8;
};

class reliability_distributions_calculator4 : public test_schedule_setup {
public:
  reliability_distributions_calculator4()
      : test_schedule_setup("modules/reliability/resources/schedule4/",
                            "20151019") {}
  short const RE_F_L_D = 1;
};

TEST_F(reliability_distributions_calculator, is_pre_computed_train) {
  ASSERT_TRUE(precomputation::detail::is_pre_computed_route(
      *schedule_, *graph_accessor::get_first_route_node(*schedule_, IC_DA_H)));
  ASSERT_TRUE(precomputation::detail::is_pre_computed_route(
      *schedule_,
      *graph_accessor::get_first_route_node(*schedule_, ICE_FR_DA_H)));
  ASSERT_FALSE(precomputation::detail::is_pre_computed_route(
      *schedule_, *graph_accessor::get_first_route_node(*schedule_, RE_K_S)));
}

void test_distributions(
    node const& route_node,
    distributions_container::container& precomputed_distributions,
    bool const pre_computed_distributions) {
  auto const route_edge = graph_accessor::get_departing_route_edge(route_node);
  // last route node
  if (route_edge == nullptr) {
    return;
  }
  auto const& head_route_node = *route_edge->_to;
  for (unsigned int l = 0; l < route_edge->_m._route_edge._conns.size(); l++) {
    auto const dep_key = distributions_container::to_container_key(
        route_edge->_m._route_edge._conns[l], route_node.get_station()->_id,
        distributions_container::container::key::departure,
        0 /* todo scheduled */);
    auto const arr_key = distributions_container::to_container_key(
        route_edge->_m._route_edge._conns[l],
        head_route_node.get_station()->_id,
        distributions_container::container::key::arrival,
        0 /* todo scheduled */);

    if (pre_computed_distributions) {
      ASSERT_TRUE(precomputed_distributions.contains_distribution(dep_key));
      auto const& departure_distribution =
          precomputed_distributions.get_distribution(dep_key);
      ASSERT_FALSE(departure_distribution.empty());
      ASSERT_TRUE(equal(departure_distribution.sum(), 1.0));

      ASSERT_TRUE(precomputed_distributions.contains_distribution(arr_key));
      auto const& arrival_distribution =
          precomputed_distributions.get_distribution(arr_key);
      ASSERT_FALSE(arrival_distribution.empty());
      ASSERT_TRUE(equal(arrival_distribution.sum(), 1.0));
    } else {
      ASSERT_FALSE(precomputed_distributions.contains_distribution(dep_key));
      ASSERT_FALSE(precomputed_distributions.contains_distribution(arr_key));
    }
  }

  test_distributions(head_route_node, precomputed_distributions,
                     pre_computed_distributions);
}

TEST_F(reliability_distributions_calculator, Initial_distributions_simple) {
  distributions_container::container precomputed_distributions;
  start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                        {0.1, 0.8, 0.1}, -1);

  precomputation::perform_precomputation(*schedule_, s_t_distributions,
                                         precomputed_distributions);

  for (auto const first_route_node :
       schedule_->route_index_to_first_route_node) {
    test_distributions(*first_route_node, precomputed_distributions,
                       precomputation::detail::is_pre_computed_route(
                           *schedule_, *first_route_node));
  }
}

#if 0
#include "motis/reliability/db_distributions.h"
TEST_F(reliability_distributions_calculator, Initial_distributions_db_distributions) {
  distributions_container::container
      precomputed_distributions(schedule_->node_count);
  db_distributions db_dists(
      "/home/keyhani/Workspace/git/motis/DBDists/DBData/20130805/Original/td/", 120,
      120);  // todo: read max travel time from graph

  precomputation::perform_precomputation(
      *schedule_, db_dists, precomputed_distributions);

  for (auto const first_route_node :
       schedule_->route_index_to_first_route_node) {
    test_distributions(
        *first_route_node, precomputed_distributions,
        precomputation::detail::is_pre_computed_route(
            *schedule_, *first_route_node));
  }
}
TEST_F(reliability_distributions_calculator, Initial_distributions_db_distributions2) {
  std::cout << "Initial_distributions_db_distributions2" << std::endl;
  auto schedule = loader::load_schedule("/tmp/rohdaten/rohdaten/",
                                        to_unix_time(2015, 9, 28),
                                        to_unix_time(2015, 9, 29));
  std::cout << "schedule loaded" << std::endl;
  distributions_container::container
      precomputed_distributions(schedule->node_count);
  db_distributions db_dists(
      "/home/keyhani/Workspace/git/motis/DBDists/DBData/20130805/Original/td/", 120,
      120);  // todo: read max travel time from graph

  precomputation::perform_precomputation(
      *schedule, db_dists, precomputed_distributions);

  for (auto const first_route_node :
       schedule->route_index_to_first_route_node) {
    test_distributions(
        *first_route_node, precomputed_distributions,
        precomputation::detail::is_pre_computed_route(
            *schedule, *first_route_node));
  }
}
#endif

TEST_F(reliability_distributions_calculator4, distributions_for_a_ride_RE) {
  distributions_container::container precomputed_distributions;
  start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                        {0.1, 0.8, 0.1}, -1);
  precomputation::perform_precomputation(*schedule_, s_t_distributions,
                                         precomputed_distributions);
  distributions_container::container container;

  // route node at Frankfurt of train RE_F_L_D
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule_, RE_F_L_D);
  node const& second_route_node =
      *graph_accessor::get_departing_route_edge(first_route_node)->_to;
  node const& last_route_node =
      *graph_accessor::get_departing_route_edge(second_route_node)->_to;

  ride_distribution::detail::compute_distributions_for_a_ride(
      0, last_route_node,
      context(*schedule_, precomputed_distributions, s_t_distributions),
      container);

  {
    auto const& distribution =
        container.get_distribution(distributions_container::to_container_key(
            graph_accessor::get_departing_route_edge(first_route_node)
                ->_m._route_edge._conns.front(),
            first_route_node.get_station()->_id,
            distributions_container::container::key::departure,
            0 /* todo scheduled */));
    ASSERT_FALSE(distribution.empty());
    ASSERT_TRUE(equal(distribution.sum(), 1.0));
  }
  {
    auto const& distribution =
        container.get_distribution(distributions_container::to_container_key(
            graph_accessor::get_departing_route_edge(first_route_node)
                ->_m._route_edge._conns.front(),
            second_route_node.get_station()->_id,
            distributions_container::container::key::arrival,
            0 /* todo scheduled */));
    ASSERT_FALSE(distribution.empty());
    ASSERT_TRUE(equal(distribution.sum(), 1.0));
  }
  {
    auto const& distribution =
        container.get_distribution(distributions_container::to_container_key(
            graph_accessor::get_departing_route_edge(second_route_node)
                ->_m._route_edge._conns.front(),
            second_route_node.get_station()->_id,
            distributions_container::container::key::departure,
            0 /* todo scheduled */));
    ASSERT_FALSE(distribution.empty());
    ASSERT_TRUE(equal(distribution.sum(), 1.0));
  }
  {
    auto const& distribution =
        container.get_distribution(distributions_container::to_container_key(
            graph_accessor::get_departing_route_edge(second_route_node)
                ->_m._route_edge._conns.front(),
            last_route_node.get_station()->_id,
            distributions_container::container::key::arrival,
            0 /* todo scheduled */));
    ASSERT_FALSE(distribution.empty());
    ASSERT_TRUE(equal(distribution.sum(), 1.0));
  }
}

TEST_F(reliability_distributions_calculator, distributions_for_a_ride_ICE) {
  distributions_container::container precomputed_distributions;
  start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                        {0.1, 0.8, 0.1}, -1);
  precomputation::perform_precomputation(*schedule_, s_t_distributions,
                                         precomputed_distributions);
  distributions_container::container container;

  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node =
      *graph_accessor::get_first_route_node(*schedule_, ICE_FR_DA_H);
  node const& second_route_node =
      *graph_accessor::get_departing_route_edge(first_route_node)->_to;
  node const& last_route_node =
      *graph_accessor::get_departing_route_edge(second_route_node)->_to;
  unsigned int const light_conn_idx = 1;

  ride_distribution::detail::compute_distributions_for_a_ride(
      light_conn_idx, last_route_node,
      context(*schedule_, precomputed_distributions, s_t_distributions),
      container);

  {
    auto const& distribution =
        container.get_distribution(distributions_container::to_container_key(
            graph_accessor::get_departing_route_edge(first_route_node)
                ->_m._route_edge._conns.front(),
            first_route_node.get_station()->_id,
            distributions_container::container::key::departure,
            0 /* todo scheduled */));
    ASSERT_FALSE(distribution.empty());
    ASSERT_TRUE(equal(distribution.sum(), 1.0));
  }
  {
    auto const& distribution =
        container.get_distribution(distributions_container::to_container_key(
            graph_accessor::get_departing_route_edge(first_route_node)
                ->_m._route_edge._conns.front(),
            second_route_node.get_station()->_id,
            distributions_container::container::key::arrival,
            0 /* todo scheduled */));
    ASSERT_FALSE(distribution.empty());
    ASSERT_TRUE(equal(distribution.sum(), 1.0));
  }
  {
    auto const& distribution =
        container.get_distribution(distributions_container::to_container_key(
            graph_accessor::get_departing_route_edge(second_route_node)
                ->_m._route_edge._conns.front(),
            second_route_node.get_station()->_id,
            distributions_container::container::key::departure,
            0 /* todo scheduled */));
    ASSERT_FALSE(distribution.empty());
    ASSERT_TRUE(equal(distribution.sum(), 1.0));
  }
  {
    auto const& distribution =
        container.get_distribution(distributions_container::to_container_key(
            graph_accessor::get_departing_route_edge(second_route_node)
                ->_m._route_edge._conns.front(),
            last_route_node.get_station()->_id,
            distributions_container::container::key::arrival,
            0 /* todo scheduled */));
    ASSERT_FALSE(distribution.empty());
    ASSERT_TRUE(equal(distribution.sum(), 1.0));
  }
}

TEST_F(reliability_distributions_calculator, Test_queue_element) {
  common::queue_type queue;

  node dummy_node(nullptr, 0);
  light_connection lc1(1);
  light_connection lc2(2);
  light_connection lc3(3);

  queue.emplace(&dummy_node, &dummy_node, &lc3, 0, false);
  queue.emplace(&dummy_node, &dummy_node, &lc2, 0, false);
  queue.emplace(&dummy_node, &dummy_node, &lc1, 0, false);
  queue.emplace(&dummy_node, &dummy_node, &lc3, 0, false);

  ASSERT_TRUE(queue.top().light_connection_->d_time == 1);
  queue.pop();
  ASSERT_TRUE(queue.top().light_connection_->d_time == 2);
  queue.pop();
  ASSERT_TRUE(queue.top().light_connection_->d_time == 3);
  queue.pop();

  queue.emplace(&dummy_node, &dummy_node, &lc2, 0, false);

  ASSERT_TRUE(queue.top().light_connection_->d_time == 2);
  queue.pop();
  ASSERT_TRUE(queue.top().light_connection_->d_time == 3);
  queue.pop();
  ASSERT_TRUE(queue.empty());
}

}  // namespace distributions_calculator
}  // namespace reliability
}  // namespace motis
