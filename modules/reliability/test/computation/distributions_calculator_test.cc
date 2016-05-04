#include "gtest/gtest.h"

#include "motis/loader/loader.h"

#include "motis/core/common/date_time_util.h"

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
#include "../include/test_util.h"

namespace motis {
namespace reliability {
namespace distributions_calculator {

class reliability_distributions_calculator : public test_schedule_setup {
public:
  reliability_distributions_calculator()
      : test_schedule_setup("modules/reliability/resources/schedule/",
                            "20150928") {}
  constexpr static auto DARMSTADT = "4219971";
  constexpr static auto FRANKFURT = "8351230";
  constexpr static auto HEIDELBERG = "9335048";
  /* train numbers */
  constexpr static unsigned IC_DA_H = 1;
  constexpr static unsigned IC_FR_DA = 2;
  constexpr static unsigned IC_FH_DA = 3;
  constexpr static unsigned RE_MA_DA = 4;
  constexpr static unsigned ICE_FR_DA_H = 5;
  constexpr static unsigned ICE_HA_W_HE = 6;
  constexpr static unsigned ICE_K_K = 7;
  constexpr static unsigned RE_K_S = 8;
};

class reliability_distributions_calculator4 : public test_schedule_setup {
public:
  reliability_distributions_calculator4()
      : test_schedule_setup("modules/reliability/resources/schedule4/",
                            "20151019") {}
  constexpr static unsigned RE_F_L_D = 1;
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
    bool const pre_computed_distributions, schedule const& sched) {
  auto const route_edge = graph_accessor::get_departing_route_edge(route_node);
  // last route node
  if (route_edge == nullptr) {
    return;
  }
  auto const& head_route_node = *route_edge->to_;
  for (unsigned int l = 0; l < route_edge->m_.route_edge_.conns_.size(); l++) {
    auto const dep_key = distributions_container::to_container_key(
        route_node, route_edge->m_.route_edge_.conns_[l], time_util::departure,
        sched);
    auto const arr_key = distributions_container::to_container_key(
        head_route_node, route_edge->m_.route_edge_.conns_[l],
        time_util::arrival, sched);

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
                     pre_computed_distributions, sched);
}

TEST_F(reliability_distributions_calculator, Initial_distributions_simple) {
  distributions_container::container precomputed_distributions;
  start_and_travel_test_distributions s_t_distributions({0.8, 0.2},
                                                        {0.1, 0.8, 0.1}, -1);

  precomputation::perform_precomputation(*schedule_, s_t_distributions,
                                         precomputed_distributions);

  for (auto const first_route_node :
       schedule_->route_index_to_first_route_node_) {
    test_distributions(*first_route_node, precomputed_distributions,
                       precomputation::detail::is_pre_computed_route(
                           *schedule_, *first_route_node),
                       *schedule_);
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
      *graph_accessor::get_departing_route_edge(first_route_node)->to_;
  node const& last_route_node =
      *graph_accessor::get_departing_route_edge(second_route_node)->to_;

  ride_distribution::detail::compute_distributions_for_a_ride(
      0, last_route_node,
      context(*schedule_, precomputed_distributions, s_t_distributions),
      container);

  {
    auto const& distribution =
        container.get_distribution(distributions_container::to_container_key(
            first_route_node,
            graph_accessor::get_departing_route_edge(first_route_node)
                ->m_.route_edge_.conns_.front(),
            time_util::departure, *schedule_));
    ASSERT_FALSE(distribution.empty());
    ASSERT_TRUE(equal(distribution.sum(), 1.0));
  }
  {
    auto const& distribution =
        container.get_distribution(distributions_container::to_container_key(
            second_route_node,
            graph_accessor::get_departing_route_edge(first_route_node)
                ->m_.route_edge_.conns_.front(),
            time_util::arrival, *schedule_));
    ASSERT_FALSE(distribution.empty());
    ASSERT_TRUE(equal(distribution.sum(), 1.0));
  }
  {
    auto const& distribution =
        container.get_distribution(distributions_container::to_container_key(
            second_route_node,
            graph_accessor::get_departing_route_edge(second_route_node)
                ->m_.route_edge_.conns_.front(),
            time_util::departure, *schedule_));
    ASSERT_FALSE(distribution.empty());
    ASSERT_TRUE(equal(distribution.sum(), 1.0));
  }
  {
    auto const& distribution =
        container.get_distribution(distributions_container::to_container_key(
            last_route_node,
            graph_accessor::get_departing_route_edge(second_route_node)
                ->m_.route_edge_.conns_.front(),
            time_util::arrival, *schedule_));
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
      *graph_accessor::get_departing_route_edge(first_route_node)->to_;
  node const& last_route_node =
      *graph_accessor::get_departing_route_edge(second_route_node)->to_;
  unsigned int const light_conn_idx = 1;

  ride_distribution::detail::compute_distributions_for_a_ride(
      light_conn_idx, last_route_node,
      context(*schedule_, precomputed_distributions, s_t_distributions),
      container);

  {
    auto const& distribution =
        container.get_distribution(distributions_container::to_container_key(
            first_route_node,
            graph_accessor::get_departing_route_edge(first_route_node)
                ->m_.route_edge_.conns_[light_conn_idx],
            time_util::departure, *schedule_));
    ASSERT_FALSE(distribution.empty());
    ASSERT_TRUE(equal(distribution.sum(), 1.0));
  }
  {
    auto const& distribution =
        container.get_distribution(distributions_container::to_container_key(
            second_route_node,
            graph_accessor::get_departing_route_edge(first_route_node)
                ->m_.route_edge_.conns_[light_conn_idx],
            time_util::arrival, *schedule_));
    ASSERT_FALSE(distribution.empty());
    ASSERT_TRUE(equal(distribution.sum(), 1.0));
  }
  {
    auto const& distribution =
        container.get_distribution(distributions_container::to_container_key(
            second_route_node,
            graph_accessor::get_departing_route_edge(second_route_node)
                ->m_.route_edge_.conns_[light_conn_idx],
            time_util::departure, *schedule_));
    ASSERT_FALSE(distribution.empty());
    ASSERT_TRUE(equal(distribution.sum(), 1.0));
  }
  {
    auto const& distribution =
        container.get_distribution(distributions_container::to_container_key(
            last_route_node,
            graph_accessor::get_departing_route_edge(second_route_node)
                ->m_.route_edge_.conns_[light_conn_idx],
            time_util::arrival, *schedule_));
    ASSERT_FALSE(distribution.empty());
    ASSERT_TRUE(equal(distribution.sum(), 1.0));
  }
}

TEST_F(reliability_distributions_calculator, get_feeder_time_interval) {
  using namespace precomputation::detail;
  bool success;
  motis::time time_begin, time_end;

  std::tie(success, time_begin, time_end) = get_feeder_time_interval(0, 5, 30);
  ASSERT_FALSE(success);

  std::tie(success, time_begin, time_end) = get_feeder_time_interval(5, 5, 30);
  ASSERT_TRUE(success);
  ASSERT_EQ(time_begin, 0);
  ASSERT_EQ(time_end, 0);

  std::tie(success, time_begin, time_end) = get_feeder_time_interval(6, 5, 30);
  ASSERT_TRUE(success);
  ASSERT_EQ(time_begin, 0);
  ASSERT_EQ(time_end, 1);

  std::tie(success, time_begin, time_end) = get_feeder_time_interval(29, 5, 30);
  ASSERT_TRUE(success);
  ASSERT_EQ(time_begin, 0);
  ASSERT_EQ(time_end, 24);

  std::tie(success, time_begin, time_end) = get_feeder_time_interval(30, 5, 30);
  ASSERT_TRUE(success);
  ASSERT_EQ(time_begin, 0);
  ASSERT_EQ(time_end, 25);

  std::tie(success, time_begin, time_end) = get_feeder_time_interval(31, 5, 30);
  ASSERT_TRUE(success);
  ASSERT_EQ(time_begin, 1);
  ASSERT_EQ(time_end, 26);

  std::tie(success, time_begin, time_end) =
      get_feeder_time_interval(2000, 5, 30);
  ASSERT_TRUE(success);
  ASSERT_EQ(time_begin, 1970);
  ASSERT_EQ(time_end, 1995);

  std::tie(success, time_begin, time_end) =
      get_feeder_time_interval(31, 31, 30);
  ASSERT_FALSE(success);

  /* at least one minute difference between feeder arrival time
   * and train departure time */
  std::tie(success, time_begin, time_end) = get_feeder_time_interval(50, 0, 30);
  ASSERT_TRUE(success);
  ASSERT_EQ(time_begin, 20);
  ASSERT_EQ(time_end, 49);

  std::tie(success, time_begin, time_end) = get_feeder_time_interval(0, 0, 1);
  ASSERT_FALSE(success);
}

TEST_F(reliability_distributions_calculator, get_feeders) {
  using namespace precomputation::detail;
  using namespace graph_accessor;
  // route node at Frankfurt of train ICE_FR_DA_H
  auto& first_route_node = *get_first_route_node(*schedule_, ICE_FR_DA_H);
  // route edge from Frankfurt to Darmstadt
  auto const first_route_edge = get_departing_route_edge(first_route_node);
  auto const& first_light_conn = first_route_edge->m_.route_edge_.conns_[0];
  auto const feeders_of_first_route_node =
      get_feeders(first_route_node, first_light_conn, *schedule_);

  ASSERT_EQ(schedule_->stations_[first_route_node.station_node_->id_]->eva_nr_,
            FRANKFURT);
  ASSERT_EQ(
      schedule_->stations_[first_route_edge->to_->station_node_->id_]->eva_nr_,
      DARMSTADT);
  ASSERT_EQ(first_light_conn.d_time_,
            test_util::minutes_to_motis_time(5 * 60 + 55));
  ASSERT_EQ(first_light_conn.a_time_,
            test_util::minutes_to_motis_time(6 * 60 + 5));
  ASSERT_EQ(feeders_of_first_route_node.size(), 0);

  // route node at Darmstadt
  auto& second_route_node = *first_route_edge->to_;
  // route edge from Darmstadt to Heidelberg
  auto& second_route_edge = *get_departing_route_edge(second_route_node);
  auto const& second_light_conn = second_route_edge.m_.route_edge_.conns_[0];
  auto all_feeders =
      get_feeders(second_route_node, second_light_conn, *schedule_);

  ASSERT_EQ(test_util::minutes_to_motis_time(6 * 60 + 11),
            second_light_conn.d_time_);
  ASSERT_EQ(2, all_feeders.size());

  std::sort(begin(all_feeders), end(all_feeders),
            [](std::pair<const node*, const light_connection*> const& lhs,
               std::pair<const node*, const light_connection*> const& rhs) {
              return lhs.second->a_time_ < rhs.second->a_time_;
            });
  {
    auto const& feeder_light_conn = *all_feeders[0].second;
    // IC_FH_DA
    ASSERT_EQ(feeder_light_conn.full_con_->con_info_->train_nr_, IC_FH_DA);
    ASSERT_EQ(feeder_light_conn.a_time_,
              test_util::minutes_to_motis_time(5 * 60 + 41));
    ASSERT_EQ(3, get_waiting_time(schedule_->waiting_time_rules_,
                                  feeder_light_conn, second_light_conn));
    ASSERT_EQ(
        all_feeders[0].first->route_,
        graph_accessor::get_first_route_node(*schedule_, IC_FH_DA)->route_);
  }
  {
    auto const& feeder_light_conn = *all_feeders[1].second;
    // IC_FH_DA
    ASSERT_EQ(feeder_light_conn.full_con_->con_info_->train_nr_, IC_FH_DA);
    ASSERT_EQ(feeder_light_conn.a_time_,
              test_util::minutes_to_motis_time(5 * 60 + 56));
    ASSERT_EQ(3, get_waiting_time(schedule_->waiting_time_rules_,
                                  feeder_light_conn, second_light_conn));
    ASSERT_EQ(
        all_feeders[1].first->route_,
        graph_accessor::get_first_route_node(*schedule_, IC_FH_DA)->route_);
  }
}

TEST_F(reliability_distributions_calculator, get_feeders_first_departure) {
  using namespace precomputation::detail;
  using namespace graph_accessor;
  // route node at Darmstadt of train IC_DA_H
  auto& first_route_node = *get_first_route_node(*schedule_, IC_DA_H);
  // route edge from Darmstadt to Heidelberg
  auto const first_route_edge = get_departing_route_edge(first_route_node);
  // journey 07:00 --> 07:28
  auto const& first_light_conn = first_route_edge->m_.route_edge_.conns_[1];
  auto all_feeders =
      get_feeders(first_route_node, first_light_conn, *schedule_);

  std::sort(all_feeders.begin(), all_feeders.end(),
            [](std::pair<node const*, light_connection const*> const& lhs,
               std::pair<node const*, light_connection const*> const& rhs) {
              return lhs.second->a_time_ < rhs.second->a_time_;
            });

  ASSERT_EQ(schedule_->stations_[first_route_node.station_node_->id_]->eva_nr_,
            DARMSTADT);
  ASSERT_EQ(
      schedule_->stations_[first_route_edge->to_->station_node_->id_]->eva_nr_,
      HEIDELBERG);
  ASSERT_EQ(first_light_conn.d_time_, test_util::minutes_to_motis_time(7 * 60));
  ASSERT_EQ(first_light_conn.a_time_,
            test_util::minutes_to_motis_time(7 * 60 + 28));
  ASSERT_EQ(2, all_feeders.size());

  {
    auto const& feeder_light_conn = *all_feeders[0].second;
    // IC_FH_DA
    ASSERT_EQ(feeder_light_conn.full_con_->con_info_->train_nr_, IC_FH_DA);
    ASSERT_EQ(feeder_light_conn.a_time_,
              test_util::minutes_to_motis_time(6 * 60 + 41));
    ASSERT_EQ(3, get_waiting_time(schedule_->waiting_time_rules_,
                                  feeder_light_conn, first_light_conn));
    ASSERT_EQ(
        graph_accessor::get_first_route_node(*schedule_, IC_FH_DA)->route_,
        all_feeders[0].first->route_);
  }
  {
    auto const& feeder_light_conn = *all_feeders[1].second;
    // IC_FR_DA
    ASSERT_EQ(feeder_light_conn.full_con_->con_info_->train_nr_, IC_FR_DA);
    ASSERT_EQ(feeder_light_conn.a_time_,
              test_util::minutes_to_motis_time(6 * 60 + 54));
    ASSERT_EQ(3, get_waiting_time(schedule_->waiting_time_rules_,
                                  feeder_light_conn, first_light_conn));
    ASSERT_EQ(
        graph_accessor::get_first_route_node(*schedule_, IC_FR_DA)->route_,
        all_feeders[1].first->route_);
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

  ASSERT_TRUE(queue.top().light_connection_->d_time_ == 1);
  queue.pop();
  ASSERT_TRUE(queue.top().light_connection_->d_time_ == 2);
  queue.pop();
  ASSERT_TRUE(queue.top().light_connection_->d_time_ == 3);
  queue.pop();

  queue.emplace(&dummy_node, &dummy_node, &lc2, 0, false);

  ASSERT_TRUE(queue.top().light_connection_->d_time_ == 2);
  queue.pop();
  ASSERT_TRUE(queue.top().light_connection_->d_time_ == 3);
  queue.pop();
  ASSERT_TRUE(queue.empty());
}

}  // namespace distributions_calculator
}  // namespace reliability
}  // namespace motis
