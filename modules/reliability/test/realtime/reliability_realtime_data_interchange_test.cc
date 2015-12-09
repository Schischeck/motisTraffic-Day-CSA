#include "gtest/gtest.h"

#include "motis/reliability/computation/data_departure_interchange.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/realtime/time_util.h"

#include "../include/interchange_data_for_tests.h"
#include "../include/message_builder.h"
#include "../include/start_and_travel_test_distributions.h"
#include "../include/test_schedule_setup.h"
#include "../include/test_util.h"

namespace motis {
namespace reliability {
using namespace time_util;
namespace calc_departure_distribution {

class reliability_realtime_data_interchange : public test_motis_setup {
public:
  reliability_realtime_data_interchange()
      : test_motis_setup("modules/reliability/resources/schedule2/", "20150928",
                         true) {}
  std::string const STUTTGART = "7309882";
  std::string const ERLANGEN = "0953067";
  std::string const KASSEL = "6380201";

  /* train numbers */
  short const ICE_S_E = 5;  // 11:32 --> 12:32
  short const ICE_E_K = 7;  // 12:45 --> 14:15
};

class reliability_realtime_data_interchange_walk : public test_motis_setup {
public:
  reliability_realtime_data_interchange_walk()
      : test_motis_setup("modules/reliability/resources/schedule3/", "20150928",
                         true) {}
  std::string const FRANKFURT = "1111111";
  std::string const MESSE = "2222222";
  std::string const LANGEN = "3333333";
  std::string const WEST = "4444444";

  short const ICE_L_H = 1;  // 10:00 --> 10:10
  short const S_M_W = 2;  // 10:20 --> 10:25
};

TEST_F(reliability_realtime_data_interchange,
       interchange_first_route_node_no_other_feeder_but_icfeeder) {
  distributions_container::container dummy;
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  bootstrap::send(
      motis_instance_,
      realtime::get_ris_message(ERLANGEN, ICE_S_E,
                                1443443520 /* 2015-09-28 12:32:00 GMT */,
                                1443444120 /* 2015-10-19 12:42:00 GMT */,
                                ris::EventType_Arrival, ris::DelayType_Is));

  // arriving train ICE_S_E from Stuttgart to Erlangen
  // interchange at Stuttgart
  // departing train ICE_E_K from Erlangen to Kassel
  interchange_data_for_tests const ic_data(
      get_schedule(), ICE_S_E, ICE_E_K, STUTTGART, ERLANGEN, KASSEL,
      11 * 60 + 32, 12 * 60 + 42, 12 * 60 + 47, 14 * 60 + 17);

  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  data_departure_interchange data(
      true, ic_data.tail_node_departing_train_,
      *ic_data.arriving_route_edge_._to, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, dummy_arrival_distribution, dummy,
      context(get_schedule(), dummy, s_t_distributions));

  ASSERT_EQ(test_util::minutes_to_motis_time(12 * 60 + 45),
            data.scheduled_departure_time_);
  ASSERT_EQ(3, data.largest_delay());

  ASSERT_EQ(3, data.maximum_waiting_time_);
  ASSERT_EQ(test_util::minutes_to_motis_time(12 * 60 + 32),
            data.interchange_feeder_info_.scheduled_arrival_time_);
  ASSERT_EQ(&dummy_arrival_distribution,
            data.interchange_feeder_info_.arrival_distribution_);
  ASSERT_EQ(get_schedule()
                .stations[ic_data.tail_node_departing_train_._station_node->_id]
                ->transfer_time,
            data.interchange_feeder_info_.transfer_time_);
  ASSERT_EQ(3, data.interchange_feeder_info_.waiting_time_);
  ASSERT_EQ((test_util::minutes_to_motis_time(12 * 60 + 45) + 3) - 5,
            data.interchange_feeder_info_.latest_feasible_arrival_);
}

TEST_F(reliability_realtime_data_interchange_walk, interchange_walk) {
  distributions_container::container dummy;
  start_and_travel_test_distributions s_t_distributions({0.4, 0.4, 0.2});

  bootstrap::send(
      motis_instance_,
      realtime::get_ris_message(FRANKFURT, ICE_L_H,
                                1443435000 /* 2015-09-28 10:10:00 GMT */,
                                1443435060 /* 2015-10-19 10:11:00 GMT */,
                                ris::EventType_Arrival, ris::DelayType_Is));

  // arriving train ICE_L_H from Langen to Frankfurt
  // interchange at Frankfurt and walking to Messe
  // departing train S_M_W from Messe to West
  interchange_data_for_tests const ic_data(
      get_schedule(), ICE_L_H, S_M_W, LANGEN, FRANKFURT, MESSE, WEST, 10 * 60,
      10 * 60 + 11, 10 * 60 + 20, 10 * 60 + 25);

  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(1, 1.0);

  data_departure_interchange_walk data(
      true, ic_data.tail_node_departing_train_,
      *ic_data.arriving_route_edge_._to, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, dummy_arrival_distribution, dummy,
      context(get_schedule(), dummy, s_t_distributions));

  ASSERT_EQ(test_util::minutes_to_motis_time(10 * 60 + 20),
            data.scheduled_departure_time_);
  ASSERT_EQ(2, data.largest_delay());

  ASSERT_EQ(test_util::minutes_to_motis_time(10 * 60 + 10),
            data.interchange_feeder_info_.scheduled_arrival_time_);
  ASSERT_EQ(&dummy_arrival_distribution,
            data.interchange_feeder_info_.arrival_distribution_);
  ASSERT_EQ(test_util::minutes_to_motis_time(10 * 60 + 20) - 10,
            data.interchange_feeder_info_.latest_feasible_arrival_);
}
}
}
}
