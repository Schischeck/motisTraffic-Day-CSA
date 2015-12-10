#include "gtest/gtest.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/realtime/time_util.h"
#include "motis/reliability/realtime/dependencies_finder.h"

#include "../include/message_builder.h"
#include "../include/start_and_travel_test_distributions.h"
#include "../include/test_schedule_setup.h"
#include "../include/test_util.h"

namespace motis {
namespace reliability {
namespace realtime {
namespace dependencies_finder {

class reliability_realtime_dependencies : public test_motis_setup {
public:
  reliability_realtime_dependencies()
      : test_motis_setup(
            "modules/reliability/resources/schedule_realtime_update/",
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

/** Arrival of RE_H_F at 09:25 depends on the
 * departure of RE_H_F at 09:15 */
TEST_F(reliability_realtime_dependencies, RE_H_F_dep) {
  /* route node in Hanau */
  auto const& node =
      *graph_accessor::get_first_route_node(get_schedule(), RE_H_F);
  auto const& light_conn =
      graph_accessor::get_departing_route_edge(node)->_m._route_edge._conns[0];

  auto const dependencies =
      find_dependencies(distributions_container::to_container_key(
          node, light_conn, time_util::departure, get_schedule()));
  ASSERT_EQ(1, dependencies.size());
  auto const& dependency = dependencies.front();

  ASSERT_EQ(graph_accessor::get_departing_route_edge(node)->_to,
            dependency.route_node_);
  ASSERT_EQ(&light_conn, dependency.light_connection_);
  ASSERT_EQ(time_util::arrival, dependency.type_);
}

/** No events depends on the arrival of RE_H_F at 09:25 in Frankfurt */
TEST_F(reliability_realtime_dependencies, RE_H_F_arr) {
  /* route node in frankfurt */
  auto const& node =
      *graph_accessor::get_departing_route_edge(
           *graph_accessor::get_first_route_node(get_schedule(), RE_H_F))
           ->_to;
  auto const& light_conn =
      graph_accessor::get_arriving_route_edge(node)->_m._route_edge._conns[0];

  ASSERT_TRUE(find_dependencies(
                  distributions_container::to_container_key(
                      node, light_conn, time_util::arrival, get_schedule()))
                  .empty());
}

/**
 * Departure of ICE_F_H at 09:10 in Frankfurt depends on the
 * arrival of ICE_D_L_F at 09:00 in Frankfurt */
void test_ICE_D_L_F_Frankfurt(reliability_realtime_dependencies& test_info,
                              unsigned const actual_arrival_time) {
  /* route node of ICE_D_L_F at Frankfurt */
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

  auto const dependencies =
      find_dependencies(distributions_container::to_container_key(
          node, light_conn, time_util::arrival, test_info.get_schedule()));
  ASSERT_EQ(1, dependencies.size());

  auto const& dependent_node = *graph_accessor::get_first_route_node(
      test_info.get_schedule(), test_info.ICE_F_H);
  auto const& dependent_light_conn =
      graph_accessor::get_departing_route_edge(dependent_node)
          ->_m._route_edge._conns[0];

  auto const& dependency = dependencies.front();
  ASSERT_EQ(&dependent_node, dependency.route_node_);
  ASSERT_EQ(&dependent_light_conn, dependency.light_connection_);
  ASSERT_EQ(time_util::departure, dependency.type_);
}

/** Departure of ICE_F_H at 09:10 in Frankfurt depends on the
 * arrival of ICE_D_L_F at 09:00 in Frankfurt.
 * Additionally, ensure that forecasts and propagations
 * have no effect on the dependencies. */
TEST_F(reliability_realtime_dependencies, ICE_D_L_F_Frankfurt) {
  test_ICE_D_L_F_Frankfurt(*this, 9 * 60);
  bootstrap::send(
      motis_instance_,
      realtime::get_delay_message(
          FRANKFURT, ICE_D_L_F, 1445245200 /* 2015-10-19 09:00:00 GMT */,
          1445248800 /* 2015-10-19 10:00:00 GMT */, ris::EventType_Arrival,
          ris::DelayType_Forecast));
  test_ICE_D_L_F_Frankfurt(*this, 10 * 60);
  bootstrap::send(
      motis_instance_,
      realtime::get_delay_message(
          FRANKFURT, ICE_D_L_F, 1445245200 /* 2015-10-19 09:00:00 GMT */,
          1445247000 /* 2015-10-19 09:30:00 GMT */, ris::EventType_Arrival,
          ris::DelayType_Forecast));
  test_ICE_D_L_F_Frankfurt(*this, 9 * 60 + 30);
  bootstrap::send(
      motis_instance_,
      realtime::get_delay_message(
          LANGEN, ICE_D_L_F, 1445244720 /* 2015-10-19 08:52:00 GMT */,
          1445247000 /* 2015-10-19 09:30:00 GMT */, ris::EventType_Departure,
          ris::DelayType_Forecast));
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
  bootstrap::send(motis_instance_, realtime::get_cancel_message(events));

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

  auto const dependencies = find_dependencies(
      {ICE_D_L_F, "ICE", "",
       get_schedule().eva_to_station.find("LANGEN")->second->index,
       time_util::arrival, test_util::minutes_to_motis_time(9 * 60)});
  ASSERT_EQ(1, dependencies.size());

  auto const& dependent_node =
      *graph_accessor::get_first_route_node(get_schedule(), ICE_F_H);
  auto const& dependent_light_conn =
      graph_accessor::get_departing_route_edge(dependent_node)
          ->_m._route_edge._conns[0];

  auto const& dependency = dependencies.front();
  ASSERT_EQ(&dependent_node, dependency.route_node_);
  ASSERT_EQ(&dependent_light_conn, dependency.light_connection_);
  ASSERT_EQ(time_util::departure, dependency.type_);
}

/** Departure of ICE_F_H at 09:10 in Frankfurt depends on the
 * arrival of ICE_D_L_F at 09:00 in Frankfurt */
TEST_F(reliability_realtime_dependencies, reroute_cancellation) {
  std::vector<realtime::event> events;
  events.push_back({LANGEN, ICE_D_L_F, 1445244720, ris::EventType_Departure});
  events.push_back({FRANKFURT, ICE_D_L_F, 1445245200, ris::EventType_Arrival});
  bootstrap::send(motis_instance_, realtime::get_reroute_message(events, {}));

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

  auto const dependencies = find_dependencies(
      {ICE_D_L_F, "ICE", "",
       get_schedule().eva_to_station.find("LANGEN")->second->index,
       time_util::arrival, test_util::minutes_to_motis_time(9 * 60)});
  ASSERT_EQ(1, dependencies.size());

  auto const& dependent_node =
      *graph_accessor::get_first_route_node(get_schedule(), ICE_F_H);
  auto const& dependent_light_conn =
      graph_accessor::get_departing_route_edge(dependent_node)
          ->_m._route_edge._conns[0];

  auto const& dependency = dependencies.front();
  ASSERT_EQ(&dependent_node, dependency.route_node_);
  ASSERT_EQ(&dependent_light_conn, dependency.light_connection_);
  ASSERT_EQ(time_util::departure, dependency.type_);
}

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
  bootstrap::send(motis_instance_, realtime::get_reroute_message({}, events));

  /* Ensure that arrival at 16:10 is added */
  auto const& all_connections =
      graph_accessor::get_departing_route_edge(
          *graph_accessor::get_first_route_node(get_schedule(), ICE_D_F))
          ->_m._route_edge._conns;
  ASSERT_EQ(1, all_connections.size());
  ASSERT_EQ(test_util::minutes_to_motis_time(16 * 60 + 10),
            all_connections[0].a_time);

  auto const dependencies = find_dependencies(
      {ICE_D_F, "ICE", "",
       get_schedule().eva_to_station.find("LANGEN")->second->index,
       time_util::arrival, test_util::minutes_to_motis_time(16 * 10)});
  ASSERT_EQ(1, dependencies.size());

  auto const& dependent_node =
      *graph_accessor::get_first_route_node(get_schedule(), ICE_L_H_16);
  auto const& dependent_light_conn =
      graph_accessor::get_departing_route_edge(dependent_node)
          ->_m._route_edge._conns[0];

  auto const& dependency = dependencies.front();
  ASSERT_EQ(&dependent_node, dependency.route_node_);
  ASSERT_EQ(&dependent_light_conn, dependency.light_connection_);
  ASSERT_EQ(time_util::departure, dependency.type_);
}

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

  ASSERT_TRUE(find_dependencies(
                  distributions_container::to_container_key(
                      node, light_conn, time_util::arrival, get_schedule()))
                  .empty());
}

/** The following events depend on the
 * arrival of ICE_D_L_F at 08:30 in Langen:
 * Departure of ICE_D_L_F at 08:32 in Langen (stay in train)
 * Departure of ICE_D_L_F at 08:52 in Langen
 * Departure of ICE_L_H at 08:50 in Langen */
TEST_F(reliability_realtime_dependencies, ICE_D_L_F_Langen) {
  /* route node in Langen */
  auto const& node =
      *graph_accessor::get_departing_route_edge(
           *graph_accessor::get_first_route_node(get_schedule(), ICE_D_L_F))
           ->_to;
  auto const& light_conn =
      graph_accessor::get_arriving_route_edge(node)->_m._route_edge._conns[1];

  auto dependencies =
      find_dependencies(distributions_container::to_container_key(
          node, light_conn, time_util::arrival, get_schedule()));
  ASSERT_EQ(3, dependencies.size());

  std::sort(dependencies.begin(), dependencies.end(),
            [](dependency const& lhs, dependency const& rhs) {
              return lhs.light_connection_->d_time <
                     rhs.light_connection_->d_time;
            });

  {
    /* Departure of ICE_D_L_F at 08:32 in Langen (stay in train) */
    auto const& dependent_node =
        *graph_accessor::get_departing_route_edge(
             *graph_accessor::get_first_route_node(get_schedule(), ICE_D_L_F))
             ->_to;
    auto const& dependent_light_conn =
        graph_accessor::get_departing_route_edge(dependent_node)
            ->_m._route_edge._conns[1];

    auto const& dependency = dependencies.front();
    ASSERT_EQ(&dependent_node, dependency.route_node_);
    ASSERT_EQ(&dependent_light_conn, dependency.light_connection_);
    ASSERT_EQ(time_util::departure, dependency.type_);
  }
  {
    /* Departure of ICE_L_H at 08:50 in Langen */
    auto const& dependent_node =
        *graph_accessor::get_first_route_node(get_schedule(), ICE_L_H);
    auto const& dependent_light_conn =
        graph_accessor::get_departing_route_edge(dependent_node)
            ->_m._route_edge._conns[1];

    auto const& dependency = dependencies.front();
    ASSERT_EQ(&dependent_node, dependency.route_node_);
    ASSERT_EQ(&dependent_light_conn, dependency.light_connection_);
    ASSERT_EQ(time_util::departure, dependency.type_);
  }
  {
    /* Departure of ICE_D_L_F at 08:52 in Langen */
    auto const& dependent_node =
        *graph_accessor::get_departing_route_edge(
             *graph_accessor::get_first_route_node(get_schedule(), ICE_D_L_F))
             ->_to;
    auto const& dependent_light_conn =
        graph_accessor::get_departing_route_edge(dependent_node)
            ->_m._route_edge._conns[2];

    auto const& dependency = dependencies.front();
    ASSERT_EQ(&dependent_node, dependency.route_node_);
    ASSERT_EQ(&dependent_light_conn, dependency.light_connection_);
    ASSERT_EQ(time_util::departure, dependency.type_);
  }
}

/** Arrival of ICE_D_L_F at 08:30 in Langen
 * is delayed to 08:50. The following events depend on it:
 * Departure of ICE_D_L_F at 08:32 in Langen (stay in train)
 * Departure of ICE_D_L_F at 08:52 in Langen
 * Departure of ICE_L_H at 08:50 in Langen.
 * Additionally, the following event depends on it:
 * Departure of ICE_L_H at 09:20 in Langen.
 * */
TEST_F(reliability_realtime_dependencies, ICE_D_L_F_Langen_delay) {
  bootstrap::send(
      motis_instance_,
      realtime::get_delay_message(LANGEN, ICE_D_L_F,
                                  1445243400 /* 2015-10-19 08:30:00 GMT */,
                                  1445244600 /* 2015-10-19 08:50:00 GMT */,
                                  ris::EventType_Arrival, ris::DelayType_Is));
  /* route node in Langen */
  auto const& node =
      *graph_accessor::get_departing_route_edge(
           *graph_accessor::get_first_route_node(get_schedule(), ICE_D_L_F))
           ->_to;
  auto const& light_conn =
      graph_accessor::get_arriving_route_edge(node)->_m._route_edge._conns[1];

  auto dependencies =
      find_dependencies(distributions_container::to_container_key(
          node, light_conn, time_util::arrival, get_schedule()));
  ASSERT_EQ(3, dependencies.size());

  std::sort(dependencies.begin(), dependencies.end(),
            [](dependency const& lhs, dependency const& rhs) {
              return lhs.light_connection_->d_time <
                     rhs.light_connection_->d_time;
            });

  {
    /* Departure of ICE_D_L_F at 08:32 in Langen (stay in train) */
    auto const& dependent_node =
        *graph_accessor::get_departing_route_edge(
             *graph_accessor::get_first_route_node(get_schedule(), ICE_D_L_F))
             ->_to;
    auto const& dependent_light_conn =
        graph_accessor::get_departing_route_edge(dependent_node)
            ->_m._route_edge._conns[1];

    auto const& dependency = dependencies.front();
    ASSERT_EQ(&dependent_node, dependency.route_node_);
    ASSERT_EQ(&dependent_light_conn, dependency.light_connection_);
    ASSERT_EQ(time_util::departure, dependency.type_);
  }
  {
    /* Departure of ICE_L_H at 08:50 in Langen */
    auto const& dependent_node =
        *graph_accessor::get_first_route_node(get_schedule(), ICE_L_H);
    auto const& dependent_light_conn =
        graph_accessor::get_departing_route_edge(dependent_node)
            ->_m._route_edge._conns[1];

    auto const& dependency = dependencies.front();
    ASSERT_EQ(&dependent_node, dependency.route_node_);
    ASSERT_EQ(&dependent_light_conn, dependency.light_connection_);
    ASSERT_EQ(time_util::departure, dependency.type_);
  }
  {
    /* Departure of ICE_D_L_F at 08:52 in Langen */
    auto const& dependent_node =
        *graph_accessor::get_departing_route_edge(
             *graph_accessor::get_first_route_node(get_schedule(), ICE_D_L_F))
             ->_to;
    auto const& dependent_light_conn =
        graph_accessor::get_departing_route_edge(dependent_node)
            ->_m._route_edge._conns[2];

    auto const& dependency = dependencies.front();
    ASSERT_EQ(&dependent_node, dependency.route_node_);
    ASSERT_EQ(&dependent_light_conn, dependency.light_connection_);
    ASSERT_EQ(time_util::departure, dependency.type_);
  }
  {
    /* Departure of ICE_L_H at 09:20 in Langen */
    auto const& dependent_node =
        *graph_accessor::get_first_route_node(get_schedule(), ICE_L_H);
    auto const& dependent_light_conn =
        graph_accessor::get_departing_route_edge(dependent_node)
            ->_m._route_edge._conns[2];

    auto const& dependency = dependencies.front();
    ASSERT_EQ(&dependent_node, dependency.route_node_);
    ASSERT_EQ(&dependent_light_conn, dependency.light_connection_);
    ASSERT_EQ(time_util::departure, dependency.type_);
  }
}
}
}
}
}
