#include "gtest/gtest.h"

#include "motis/reliability/distributions/distributions_container.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/realtime/time_util.h"

#include "../include/message_builder.h"
#include "../include/schedules/schedule_realtime_dependencies.h"
#include "../include/start_and_travel_test_distributions.h"
#include "../include/test_schedule_setup.h"
#include "../include/test_util.h"

namespace motis {
namespace reliability {
namespace realtime {

using namespace module;

class reliability_realtime_dependencies : public test_motis_setup {
public:
  reliability_realtime_dependencies()
      : test_motis_setup(schedule_realtime_dependencies::PATH,
                         schedule_realtime_dependencies::DATE, true) {}
};

/** No entry for RE events (day do not wait) */
TEST_F(reliability_realtime_dependencies, no_entry) {
  /* route node in frankfurt */
  auto const& node =
      *graph_accessor::get_departing_route_edge(
           *graph_accessor::get_first_route_node(
               get_schedule(), schedule_realtime_dependencies::RE_H_F))
           ->to_;
  auto const& light_conn =
      graph_accessor::get_arriving_route_edge(node)->m_.route_edge_.conns_[0];

  ASSERT_TRUE(get_reliability_module()
                  .precomputed_distributions()
                  .get_node(distributions_container::to_container_key(
                      node, light_conn, event_type::ARR, get_schedule()))
                  .pd_.empty());
}

/* test predecessors of Departure of ICE_F_H at 09:10 in Frankfurt:
 * arrival of ICE_D_L_F at 08:40 in Frankfurt and
 * arrival of ICE_D_L_F at 09:00 in Frankfurt */
void test_predecessors_ice_ffm_hanau_in_ffm(
    reliability_realtime_dependencies& test_info,
    distributions_container::container::node const& distribution_node_dep) {
  auto predecessors = distribution_node_dep.predecessors_;
  std::sort(predecessors.begin(), predecessors.end(),
            [](distributions_container::container::node const* lhs,
               distributions_container::container::node const* rhs) {
              return lhs->key_.scheduled_event_time_ <
                     rhs->key_.scheduled_event_time_;
            });
  ASSERT_EQ(2, predecessors.size());
  {
    /* arrival of ICE_D_L_F at 08:40 in Frankfurt */
    auto const& distribution_node_arr = *predecessors[0];
    auto const& node_arr =
        *graph_accessor::get_departing_route_edge(
             *graph_accessor::get_departing_route_edge(
                  *graph_accessor::get_first_route_node(
                      test_info.get_schedule(),
                      schedule_realtime_dependencies::ICE_D_L_F))
                  ->to_)
             ->to_;
    auto const& lc_arr = graph_accessor::get_arriving_route_edge(node_arr)
                             ->m_.route_edge_.conns_[1];
    ASSERT_EQ(distributions_container::to_container_key(
                  node_arr, lc_arr, event_type::ARR, test_info.get_schedule()),
              distribution_node_arr.key_);

    ASSERT_EQ(1, distribution_node_arr.successors_.size());
    ASSERT_EQ(&distribution_node_dep,
              distribution_node_arr.successors_.front());
  }
  {
    /* arrival ofschedule_realtime_dependencies::ICE_D_L_F at 09:00 in Frankfurt
     */
    auto const& distribution_node_arr = *predecessors[1];
    auto const& node_arr =
        *graph_accessor::get_departing_route_edge(
             *graph_accessor::get_departing_route_edge(
                  *graph_accessor::get_first_route_node(
                      test_info.get_schedule(),
                      schedule_realtime_dependencies::ICE_D_L_F))
                  ->to_)
             ->to_;
    auto const& lc_arr = graph_accessor::get_arriving_route_edge(node_arr)
                             ->m_.route_edge_.conns_[2];
    ASSERT_EQ(distributions_container::to_container_key(
                  node_arr, lc_arr, event_type::ARR, test_info.get_schedule()),
              distribution_node_arr.key_);

    ASSERT_EQ(1, distribution_node_arr.successors_.size());
    ASSERT_EQ(&distribution_node_dep,
              distribution_node_arr.successors_.front());
  }
}

/**
 * Departure of ICE_F_H at 09:10 in Frankfurt depends on
 * arrival ofschedule_realtime_dependencies::ICE_D_L_F at 08:40 in Frankfurt and
 * arrival ofschedule_realtime_dependencies::ICE_D_L_F at 09:00 in Frankfurt */
void test_ice_darm_langen_ffm_in_ffm(
    reliability_realtime_dependencies& test_info,
    unsigned const actual_arrival_time) {
  /* arrival node ofschedule_realtime_dependencies::ICE_D_L_F at Frankfurt */
  auto const& node = *graph_accessor::get_departing_route_edge(
                          *graph_accessor::get_departing_route_edge(
                               *graph_accessor::get_first_route_node(
                                   test_info.get_schedule(),
                                   schedule_realtime_dependencies::ICE_D_L_F))
                               ->to_)
                          ->to_;
  auto const& light_conn =
      graph_accessor::get_arriving_route_edge(node)->m_.route_edge_.conns_[2];

  ASSERT_EQ(test_util::minutes_to_motis_time(actual_arrival_time),
            light_conn.a_time_);

  auto const& distribution_node_arr =
      test_info.get_reliability_module().precomputed_distributions().get_node(
          distributions_container::to_container_key(
              node, light_conn, event_type::ARR, test_info.get_schedule()));

  auto const& node_dep = *graph_accessor::get_first_route_node(
      test_info.get_schedule(), schedule_realtime_dependencies::ICE_F_H);
  auto const& lc_dep = graph_accessor::get_departing_route_edge(node_dep)
                           ->m_.route_edge_.conns_[0];

  ASSERT_EQ(1, distribution_node_arr.successors_.size());
  auto const& distribution_node_dep =
      *distribution_node_arr.successors_.front();
  ASSERT_EQ(distributions_container::to_container_key(
                node_dep, lc_dep, event_type::DEP, test_info.get_schedule()),
            distribution_node_dep.key_);

  test_predecessors_ice_ffm_hanau_in_ffm(test_info, distribution_node_dep);
}

/** Departure of ICE_F_H at 09:10 in Frankfurt depends on the
 * arrival ofschedule_realtime_dependencies::ICE_D_L_F at 09:00 in Frankfurt.
 * Additionally, ensure that forecasts and propagations
 * have no effect on the dependencies. */
TEST_F(reliability_realtime_dependencies, ICE_D_L_F_Frankfurt) {
  test_ice_darm_langen_ffm_in_ffm(*this, 9 * 60);

  publish(realtime::get_delay_message(
      schedule_realtime_dependencies::FRANKFURT,
      schedule_realtime_dependencies::ICE_D_L_F, "", EventType_ARR,
      1445238000 /* 2015-10-19 09:00:00 GMT+2:00 */,
      1445241600 /* 2015-10-19 10:00:00 GMT+2:00 */,
      schedule_realtime_dependencies::DARMSTADT,
      schedule_realtime_dependencies::ICE_D_L_F,
      1445236800 /* 2015-10-19 08:40:00 GMT+2:00 */, ris::DelayType_Forecast));
  publish(make_no_msg("/ris/system_time_changed"));

  test_ice_darm_langen_ffm_in_ffm(*this, 10 * 60);

  publish(realtime::get_delay_message(
      schedule_realtime_dependencies::FRANKFURT,
      schedule_realtime_dependencies::ICE_D_L_F, "", EventType_ARR,
      1445238000 /* 2015-10-19 09:00:00 GMT+2:00 */,
      1445239800 /* 2015-10-19 09:30:00 GMT+2:00 */,
      schedule_realtime_dependencies::DARMSTADT,
      schedule_realtime_dependencies::ICE_D_L_F,
      1445236800 /* 2015-10-19 08:40:00 GMT+2:00 */, ris::DelayType_Forecast));
  publish(make_no_msg("/ris/system_time_changed"));

  test_ice_darm_langen_ffm_in_ffm(*this, 9 * 60 + 30);

  publish(realtime::get_delay_message(
      schedule_realtime_dependencies::LANGEN,
      schedule_realtime_dependencies::ICE_D_L_F, "", EventType_DEP,
      1445237520 /* 2015-10-19 08:52:00 GMT+2:00 */,
      1445239800 /* 2015-10-19 09:30:00 GMT+2:00 */,
      schedule_realtime_dependencies::DARMSTADT,
      schedule_realtime_dependencies::ICE_D_L_F,
      1445236800 /* 2015-10-19 08:40:00 GMT+2:00 */, ris::DelayType_Forecast));
  publish(make_no_msg("/ris/system_time_changed"));

  test_ice_darm_langen_ffm_in_ffm(*this, 9 * 60 + 38);
}

/** RE_H_F does not wait for and depend on the
 * arrival of ICE_L_H at 09:05 in Hanau */
TEST_F(reliability_realtime_dependencies, ICE_L_H) {
  /* route node in Langen */
  auto const& node =
      *graph_accessor::get_departing_route_edge(
           *graph_accessor::get_first_route_node(
               get_schedule(), schedule_realtime_dependencies::ICE_L_H))
           ->to_;
  auto const& light_conn =
      graph_accessor::get_arriving_route_edge(node)->m_.route_edge_.conns_[1];

  ASSERT_TRUE(get_reliability_module()
                  .precomputed_distributions()
                  .get_node(distributions_container::to_container_key(
                      node, light_conn, event_type::ARR, get_schedule()))
                  .successors_.empty());
}

/** Departure of ICE_L_H at 08:50 in Langen depend on the
 * arrival ofschedule_realtime_dependencies::ICE_D_L_F at 08:30 in Langen
 * Note: the is message results in creating a new route */
TEST_F(reliability_realtime_dependencies, DISABLED_ICE_D_L_F_Langen) {
  publish(realtime::get_delay_message(
      schedule_realtime_dependencies::LANGEN,
      schedule_realtime_dependencies::ICE_D_L_F, "", EventType_ARR,
      1445236200 /* 2015-10-19 08:30:00 GMT+2:00 */,
      1445237940 /* 2015-10-19 08:59:00 GMT+2:00 */,
      schedule_realtime_dependencies::DARMSTADT,
      schedule_realtime_dependencies::ICE_D_L_F,
      1445235600 /* 2015-10-19 08:20:00 GMT+2:00 */, ris::DelayType_Is));
  publish(make_no_msg("/ris/system_time_changed"));

  auto const& distribution_node_arr =
      get_reliability_module().precomputed_distributions().get_node(
          distributions_container::container::key(
              schedule_realtime_dependencies::ICE_D_L_F, "ice", "",
              get_schedule()
                  .eva_to_station_.find(schedule_realtime_dependencies::LANGEN)
                  ->second->index_,
              event_type::ARR, test_util::minutes_to_motis_time(8 * 60 + 30)));

  ASSERT_EQ(2, distribution_node_arr.successors_.size());
  {
    /* Departure ofschedule_realtime_dependencies::ICE_D_L_F at 08:32 in Langen
     */
    auto const& distribution_node_dep = *distribution_node_arr.successors_[0];
    auto const& node_dep =
        *graph_accessor::get_departing_route_edge(
             *graph_accessor::get_first_route_node(
                 get_schedule(), schedule_realtime_dependencies::ICE_D_L_F))
             ->to_;
    auto const& lc_dep = graph_accessor::get_departing_route_edge(node_dep)
                             ->m_.route_edge_.conns_[2];
    ASSERT_EQ(distributions_container::to_container_key(
                  node_dep, lc_dep, event_type::DEP, get_schedule()),
              distribution_node_dep.key_);

    ASSERT_EQ(1, distribution_node_dep.predecessors_.size());
    ASSERT_EQ(&distribution_node_arr,
              distribution_node_dep.predecessors_.front());
  }
  {
    /* Departure of ICE_L_H at 08:50 in Langen */
    auto const& distribution_node_dep = *distribution_node_arr.successors_[1];
    auto const& node_dep = *graph_accessor::get_first_route_node(
        get_schedule(), schedule_realtime_dependencies::ICE_L_H);
    auto const& lc_dep = graph_accessor::get_departing_route_edge(node_dep)
                             ->m_.route_edge_.conns_[1];
    ASSERT_EQ(distributions_container::to_container_key(
                  node_dep, lc_dep, event_type::DEP, get_schedule()),
              distribution_node_dep.key_);

    ASSERT_EQ(1, distribution_node_dep.predecessors_.size());
    ASSERT_EQ(&distribution_node_arr,
              distribution_node_dep.predecessors_.front());
  }
}
}  // namespace realtime
}  // namespace reliability
}  // namespace motis
