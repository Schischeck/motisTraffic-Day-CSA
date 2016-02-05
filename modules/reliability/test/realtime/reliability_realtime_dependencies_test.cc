#include "gtest/gtest.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/distributions/distributions_container.h"
#include "motis/reliability/realtime/time_util.h"

#include "../include/message_builder.h"
#include "../include/start_and_travel_test_distributions.h"
#include "../include/test_schedule_setup.h"
#include "../include/test_util.h"

namespace motis {
namespace reliability {
namespace realtime {

class reliability_realtime_dependencies : public test_motis_setup {
public:
  reliability_realtime_dependencies()
      : test_motis_setup(
            "modules/reliability/resources/schedule_realtime_dependencies/",
            "20151019", true) {}
  std::string const DARMSTADT = "3333333";
  std::string const FRANKFURT = "1111111";
  std::string const HANAU = "9646170";
  std::string const LANGEN = "2222222";

  /* 08:00 --> 08:10, 08:12 --> 08:20
   * 08:20 --> 08:30, 80:32 --> 08:40
   * 08:40 --> 08:50, 08:52 --> 09:00 */
  unsigned const ICE_D_L_F = 1;
  /* 08:20 --> 08:35
   * 08:50 --> 09:05
   * 09:20 --> 09:35 */
  unsigned const ICE_L_H = 2;
  /* 09:10 --> 09:20
   * 10:10 --> 10:20 */
  unsigned const ICE_F_H = 3;
  /* 09:15 --> 09:25 */
  unsigned const RE_H_F = 4;

  /* 16:00 --> 16:20 */
  unsigned const ICE_D_F = 5;
  /* 16:30 --> 16:39 */
  unsigned const ICE_L_H_16 = 6;
};

/** No entry for RE events (day do not wait) */
TEST_F(reliability_realtime_dependencies, no_entry) {
  /* route node in frankfurt */
  auto const& node =
      *graph_accessor::get_departing_route_edge(
           *graph_accessor::get_first_route_node(get_schedule(), RE_H_F))
           ->_to;
  auto const& light_conn =
      graph_accessor::get_arriving_route_edge(node)->_m._route_edge._conns[0];

  ASSERT_TRUE(get_reliability_module()
                  .precomputed_distributions()
                  .get_node(distributions_container::to_container_key(
                      node, light_conn, time_util::arrival, get_schedule()))
                  .pd_.empty());
}

/* test predecessors of Departure of ICE_F_H at 09:10 in Frankfurt:
 * arrival of ICE_D_L_F at 08:40 in Frankfurt and
 * arrival of ICE_D_L_F at 09:00 in Frankfurt */
void test_predecessors_ICE_F_H_Frankfurt(
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
                      test_info.get_schedule(), test_info.ICE_D_L_F))
                  ->_to)
             ->_to;
    auto const& lc_arr = graph_accessor::get_arriving_route_edge(node_arr)
                             ->_m._route_edge._conns[1];
    ASSERT_EQ(
        distributions_container::to_container_key(
            node_arr, lc_arr, time_util::arrival, test_info.get_schedule()),
        distribution_node_arr.key_);

    ASSERT_EQ(1, distribution_node_arr.successors_.size());
    ASSERT_EQ(&distribution_node_dep,
              distribution_node_arr.successors_.front());
  }
  {
    /* arrival of ICE_D_L_F at 09:00 in Frankfurt */
    auto const& distribution_node_arr = *predecessors[1];
    auto const& node_arr =
        *graph_accessor::get_departing_route_edge(
             *graph_accessor::get_departing_route_edge(
                  *graph_accessor::get_first_route_node(
                      test_info.get_schedule(), test_info.ICE_D_L_F))
                  ->_to)
             ->_to;
    auto const& lc_arr = graph_accessor::get_arriving_route_edge(node_arr)
                             ->_m._route_edge._conns[2];
    ASSERT_EQ(
        distributions_container::to_container_key(
            node_arr, lc_arr, time_util::arrival, test_info.get_schedule()),
        distribution_node_arr.key_);

    ASSERT_EQ(1, distribution_node_arr.successors_.size());
    ASSERT_EQ(&distribution_node_dep,
              distribution_node_arr.successors_.front());
  }
}

/**
 * Departure of ICE_F_H at 09:10 in Frankfurt depends on
 * arrival of ICE_D_L_F at 08:40 in Frankfurt and
 * arrival of ICE_D_L_F at 09:00 in Frankfurt */
void test_ICE_D_L_F_Frankfurt(reliability_realtime_dependencies& test_info,
                              unsigned const actual_arrival_time) {
  /* arrival node of ICE_D_L_F at Frankfurt */
  auto const& node =
      *graph_accessor::get_departing_route_edge(
           *graph_accessor::get_departing_route_edge(
                *graph_accessor::get_first_route_node(test_info.get_schedule(),
                                                      test_info.ICE_D_L_F))
                ->_to)
           ->_to;
  auto const& light_conn =
      graph_accessor::get_arriving_route_edge(node)->_m._route_edge._conns[2];

  ASSERT_EQ(test_util::minutes_to_motis_time(actual_arrival_time),
            light_conn.a_time);

  auto const& distribution_node_arr =
      test_info.get_reliability_module().precomputed_distributions().get_node(
          distributions_container::to_container_key(
              node, light_conn, time_util::arrival, test_info.get_schedule()));

  auto const& node_dep = *graph_accessor::get_first_route_node(
      test_info.get_schedule(), test_info.ICE_F_H);
  auto const& lc_dep = graph_accessor::get_departing_route_edge(node_dep)
                           ->_m._route_edge._conns[0];

  ASSERT_EQ(1, distribution_node_arr.successors_.size());
  auto const& distribution_node_dep =
      *distribution_node_arr.successors_.front();
  ASSERT_EQ(
      distributions_container::to_container_key(
          node_dep, lc_dep, time_util::departure, test_info.get_schedule()),
      distribution_node_dep.key_);

  test_predecessors_ICE_F_H_Frankfurt(test_info, distribution_node_dep);
}

/** Departure of ICE_F_H at 09:10 in Frankfurt depends on the
 * arrival of ICE_D_L_F at 09:00 in Frankfurt.
 * Additionally, ensure that forecasts and propagations
 * have no effect on the dependencies. */
TEST_F(reliability_realtime_dependencies, ICE_D_L_F_Frankfurt) {
  test_ICE_D_L_F_Frankfurt(*this, 9 * 60);
  test::send(motis_instance_,
             realtime::get_delay_message(
                 FRANKFURT, ICE_D_L_F, 1445245200 /* 2015-10-19 09:00:00 GMT */,
                 1445248800 /* 2015-10-19 10:00:00 GMT */,
                 ris::EventType_Arrival, ris::DelayType_Forecast));
  test_ICE_D_L_F_Frankfurt(*this, 10 * 60);
  test::send(motis_instance_,
             realtime::get_delay_message(
                 FRANKFURT, ICE_D_L_F, 1445245200 /* 2015-10-19 09:00:00 GMT */,
                 1445247000 /* 2015-10-19 09:30:00 GMT */,
                 ris::EventType_Arrival, ris::DelayType_Forecast));
  test_ICE_D_L_F_Frankfurt(*this, 9 * 60 + 30);
  test::send(motis_instance_,
             realtime::get_delay_message(
                 LANGEN, ICE_D_L_F, 1445244720 /* 2015-10-19 08:52:00 GMT */,
                 1445247000 /* 2015-10-19 09:30:00 GMT */,
                 ris::EventType_Departure, ris::DelayType_Forecast));
  test_ICE_D_L_F_Frankfurt(*this, 9 * 60 + 38);
}

/** Departure of ICE_F_H at 09:10 in Frankfurt depends on the
 * arrival of ICE_D_L_F at 09:00 in Frankfurt */
TEST_F(reliability_realtime_dependencies, cancellation) {
  std::vector<realtime::event> events;
  // 08:40 --> 08:50, 08:52 --> 09:00
  events.push_back(
      {DARMSTADT, ICE_D_L_F, 1445244000, ris::EventType_Departure});
  events.push_back({LANGEN, ICE_D_L_F, 1445244600, ris::EventType_Arrival});
  events.push_back({LANGEN, ICE_D_L_F, 1445244720, ris::EventType_Departure});
  events.push_back({FRANKFURT, ICE_D_L_F, 1445245200, ris::EventType_Arrival});
  test::send(motis_instance_, realtime::get_cancel_message(events));

  /* Ensure that 08:40 is cancelled */
  auto const& all_connections =
      graph_accessor::get_departing_route_edge(
          *graph_accessor::get_first_route_node(get_schedule(), ICE_D_L_F))
          ->_m._route_edge._conns;
  ASSERT_EQ(2, all_connections.size());
  ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60),
            all_connections[0].d_time);
  ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 20),
            all_connections[1].d_time);

  auto const node_key = distributions_container::container::key{
      ICE_D_L_F,
      "ice",
      "",
      get_schedule().eva_to_station.find(FRANKFURT)->second->index,
      time_util::arrival,
      test_util::minutes_to_motis_time(9 * 60)};
  auto const& distribution_node_arr =
      get_reliability_module().precomputed_distributions().get_node(node_key);
  ASSERT_EQ(1, distribution_node_arr.successors_.size());
  auto const& distribution_node_dep =
      *distribution_node_arr.successors_.front();

  auto const& node_dep =
      *graph_accessor::get_first_route_node(get_schedule(), ICE_F_H);
  auto const& lc_dep = graph_accessor::get_departing_route_edge(node_dep)
                           ->_m._route_edge._conns[0];

  ASSERT_EQ(distributions_container::to_container_key(
                node_dep, lc_dep, time_util::departure, get_schedule()),
            distribution_node_dep.key_);

  test_predecessors_ICE_F_H_Frankfurt(*this, distribution_node_dep);
}

/** Departure of ICE_F_H at 09:10 in Frankfurt depends on the
 * arrival of ICE_D_L_F at 09:00 in Frankfurt */
TEST_F(reliability_realtime_dependencies, reroute_cancellation) {
  std::vector<realtime::event> events;
  events.push_back({LANGEN, ICE_D_L_F, 1445244720, ris::EventType_Departure});
  events.push_back({FRANKFURT, ICE_D_L_F, 1445245200, ris::EventType_Arrival});
  test::send(motis_instance_, realtime::get_reroute_message(events, {}));

  /* Ensure that arrival at 09:00 is cancelled */
  auto const& all_connections =
      graph_accessor::get_departing_route_edge(
          *graph_accessor::get_departing_route_edge(
               *graph_accessor::get_first_route_node(get_schedule(), ICE_D_L_F))
               ->_to)
          ->_m._route_edge._conns;
  ASSERT_EQ(2, all_connections.size());
  ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 20),
            all_connections[0].a_time);
  ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 40),
            all_connections[1].a_time);

  auto const node_key = distributions_container::container::key{
      ICE_D_L_F,
      "ice",
      "",
      get_schedule().eva_to_station.find(FRANKFURT)->second->index,
      time_util::arrival,
      test_util::minutes_to_motis_time(9 * 60)};
  auto const& distribution_node_arr =
      get_reliability_module().precomputed_distributions().get_node(node_key);
  ASSERT_EQ(1, distribution_node_arr.successors_.size());
  auto const& distribution_node_dep =
      *distribution_node_arr.successors_.front();

  auto const& node_dep =
      *graph_accessor::get_first_route_node(get_schedule(), ICE_F_H);
  auto const& lc_dep = graph_accessor::get_departing_route_edge(node_dep)
                           ->_m._route_edge._conns[0];

  ASSERT_EQ(distributions_container::to_container_key(
                node_dep, lc_dep, time_util::departure, get_schedule()),
            distribution_node_dep.key_);

  test_predecessors_ICE_F_H_Frankfurt(*this, distribution_node_dep);
}

#if 0
/* Departure of ICE_L_H_16 at 16:30 in Langen depends on
 * Arrival of ICE_D_F at 16:10 in Langen (rerouted train) */
TEST_F(reliability_realtime_dependencies, reroute_additional) {
  std::vector<realtime::rerouted_event> events;
  events.emplace_back(realtime::event{LANGEN, ICE_D_F, 1445271000 /* 16:10 */,
                                      ris::EventType_Arrival},
                      "ICE", "");
  events.emplace_back(realtime::event{LANGEN, ICE_D_F, 1445271060 /* 16:11 */,
                                      ris::EventType_Departure},
                      "ICE", "");
  test::send(motis_instance_, realtime::get_reroute_message({}, events));

  /* Ensure that arrival at 16:10 is added */
  auto const& all_connections =
      graph_accessor::get_departing_route_edge(
          *graph_accessor::get_first_route_node(get_schedule(), ICE_D_F))
          ->_m._route_edge._conns;
  ASSERT_EQ(1, all_connections.size());
  ASSERT_EQ(test_util::minutes_to_motis_time(16 * 60 + 10),
            all_connections[0].a_time);

  auto const node_key = distributions_container::container::key{
      ICE_D_F,
      "ice",
      "",
      get_schedule().eva_to_station.find(LANGEN)->second->index,
      time_util::arrival,
      test_util::minutes_to_motis_time(16 * 10)};
  auto const& distribution_node_arr =
      get_reliability_module().precomputed_distributions().get_node(node_key);
  ASSERT_EQ(1, distribution_node_arr.successors_.size());
  auto const& distribution_node_dep =
      *distribution_node_arr.successors_.front();

  auto const& node_dep =
      *graph_accessor::get_first_route_node(get_schedule(), ICE_L_H_16);
  auto const& lc_dep = graph_accessor::get_departing_route_edge(node_dep)
                           ->_m._route_edge._conns[0];

  ASSERT_EQ(distributions_container::to_container_key(
                node_dep, lc_dep, time_util::departure, get_schedule()),
            distribution_node_dep.key_);

  ASSERT_EQ(1, distribution_node_dep.predecessors_.size());
  ASSERT_EQ(&distribution_node_arr,
            distribution_node_dep.predecessors_.front());
}

/* Arrival of ICE_D_F at 16:10 in Langen (rerouted train)
 * has no dependent successors since
 * departure of ICE_L_H_16 at 16:30 in Langen is cancelled. */
TEST_F(reliability_realtime_dependencies, reroute_additional_cancellation) {
  {
    std::vector<realtime::rerouted_event> events;
    events.emplace_back(realtime::event{LANGEN, ICE_D_F, 1445271000 /* 16:10 */,
                                        ris::EventType_Arrival},
                        "ICE", "");
    events.emplace_back(realtime::event{LANGEN, ICE_D_F, 1445271060 /* 16:11 */,
                                        ris::EventType_Departure},
                        "ICE", "");
    test::send(motis_instance_, realtime::get_reroute_message({}, events));

    /* Ensure that arrival at 16:10 is added */
    auto const& all_connections =
        graph_accessor::get_departing_route_edge(
            *graph_accessor::get_first_route_node(get_schedule(), ICE_D_F))
            ->_m._route_edge._conns;
    ASSERT_EQ(1, all_connections.size());
    ASSERT_EQ(test_util::minutes_to_motis_time(16 * 60 + 10),
              all_connections[0].a_time);
  }
  {
    std::vector<realtime::event> events;
    events.push_back(
        {LANGEN, ICE_L_H_16, 1445272200, ris::EventType_Departure});
    events.push_back({HANAU, ICE_L_H_16, 1445272740, ris::EventType_Arrival});
    test::send(motis_instance_, realtime::get_reroute_message(events, {}));

    /* Ensure that ICE_L_H_16 is cancelled */
    ASSERT_EQ(nullptr,
              graph_accessor::get_first_route_node(get_schedule(), ICE_L_H_16));
  }

  ASSERT_TRUE(
      get_reliability_module()
          .precomputed_distributions()
          .get_node({ICE_D_F, "ice", "",
                     get_schedule().eva_to_station.find(LANGEN)->second->index,
                     time_util::arrival,
                     test_util::minutes_to_motis_time(16 * 10)})
          .successors_.empty());
}
#endif

/** RE_H_F does not wait for and depend on the
 * arrival of ICE_L_H at 09:05 in Hanau */
TEST_F(reliability_realtime_dependencies, ICE_L_H) {
  /* route node in Langen */
  auto const& node =
      *graph_accessor::get_departing_route_edge(
           *graph_accessor::get_first_route_node(get_schedule(), ICE_L_H))
           ->_to;
  auto const& light_conn =
      graph_accessor::get_arriving_route_edge(node)->_m._route_edge._conns[1];

  ASSERT_TRUE(get_reliability_module()
                  .precomputed_distributions()
                  .get_node(distributions_container::to_container_key(
                      node, light_conn, time_util::arrival, get_schedule()))
                  .successors_.empty());
}

/** Departure of ICE_L_H at 08:50 in Langen depend on the
 * arrival of ICE_D_L_F at 08:30 in Langen
 * Note: the is message results in creating a new route */
TEST_F(reliability_realtime_dependencies, ICE_D_L_F_Langen) {
  test::send(motis_instance_,
             realtime::get_delay_message(
                 LANGEN, ICE_D_L_F, 1445243400 /* 2015-10-19 08:30:00 GMT */,
                 1445244600 /* 2015-10-19 08:59:00 GMT */,
                 ris::EventType_Arrival, ris::DelayType_Is));

  auto const& distribution_node_arr =
      get_reliability_module().precomputed_distributions().get_node(
          distributions_container::container::key(
              ICE_D_L_F, "ice", "",
              get_schedule().eva_to_station.find(LANGEN)->second->index,
              time_util::arrival,
              test_util::minutes_to_motis_time(8 * 60 + 30)));

  ASSERT_EQ(2, distribution_node_arr.successors_.size());
  {
    /* Departure of ICE_D_L_F at 08:32 in Langen */
    auto const& distribution_node_dep = *distribution_node_arr.successors_[0];
    auto const& node_dep =
        *graph_accessor::get_departing_route_edge(
             *graph_accessor::get_first_route_node(get_schedule(), ICE_D_L_F))
             ->_to;
    auto const& lc_dep = graph_accessor::get_departing_route_edge(node_dep)
                             ->_m._route_edge._conns[2];
    ASSERT_EQ(distributions_container::to_container_key(
                  node_dep, lc_dep, time_util::departure, get_schedule()),
              distribution_node_dep.key_);

    ASSERT_EQ(1, distribution_node_dep.predecessors_.size());
    ASSERT_EQ(&distribution_node_arr,
              distribution_node_dep.predecessors_.front());
  }
  {
    /* Departure of ICE_L_H at 08:50 in Langen */
    auto const& distribution_node_dep = *distribution_node_arr.successors_[1];
    auto const& node_dep =
        *graph_accessor::get_first_route_node(get_schedule(), ICE_L_H);
    auto const& lc_dep = graph_accessor::get_departing_route_edge(node_dep)
                             ->_m._route_edge._conns[1];
    ASSERT_EQ(distributions_container::to_container_key(
                  node_dep, lc_dep, time_util::departure, get_schedule()),
              distribution_node_dep.key_);

    ASSERT_EQ(1, distribution_node_dep.predecessors_.size());
    ASSERT_EQ(&distribution_node_arr,
              distribution_node_dep.predecessors_.front());
  }
}
}
}
}
