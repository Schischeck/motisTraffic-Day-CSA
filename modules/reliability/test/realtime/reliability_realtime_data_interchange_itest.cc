#include "gtest/gtest.h"

#include "motis/reliability/computation/data_departure_interchange.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/realtime/time_util.h"

#include "../include/interchange_data_for_tests.h"
#include "../include/message_builder.h"
#include "../include/schedules/schedule2.h"
#include "../include/schedules/schedule3.h"
#include "../include/start_and_travel_test_distributions.h"
#include "../include/test_schedule_setup.h"
#include "../include/test_util.h"

namespace motis {
namespace reliability {
namespace calc_departure_distribution {
using namespace time_util;
using namespace module;

constexpr auto RT_SUPPORTS_FEEDER_DEPENDENCIES = false;

class reliability_realtime_data_interchange : public test_motis_setup {
public:
  reliability_realtime_data_interchange()
      : test_motis_setup(schedule2::PATH, schedule2::DATE, true) {}
};

class reliability_realtime_data_interchange_walk : public test_motis_setup {
public:
  reliability_realtime_data_interchange_walk()
      : test_motis_setup(schedule3::PATH, schedule3::DATE, true) {}
};

TEST_F(reliability_realtime_data_interchange,
       interchange_first_route_node_no_other_feeder_but_icfeeder) {
  distributions_container::container dummy;
  distributions_container::container::node dummy_node;
  start_and_travel_test_distributions s_t_distributions({0.6, 0.4});

  publish(realtime::get_delay_message(
      schedule2::ERLANGEN.eva_, schedule2::ICE_S_E, "", EventType_ARR,
      1443436320 /* 2015-09-28 12:32:00 GMT+2:00 */,
      1443436920 /* 2015-09-28 12:42:00 GMT+2:00 */, schedule2::STUTTGART.eva_,
      schedule2::ICE_S_E, 1443432720 /* 2015-09-28 11:32:00 GMT+2:00 */,
      ris::DelayType_Is));
  publish(make_no_msg("/ris/system_time_changed"));
  // arriving train ICE_S_E from Stuttgart to Erlangen
  // interchange at Stuttgart
  // departing train ICE_E_K from Erlangen to Kassel
  interchange_data_for_tests const ic_data(
      get_schedule(), schedule2::ICE_S_E, schedule2::ICE_E_K,
      schedule2::STUTTGART.eva_, schedule2::ERLANGEN.eva_,
      schedule2::KASSEL.eva_, 11 * 60 + 32, 12 * 60 + 42,
      12 * 60 + 45 + (RT_SUPPORTS_FEEDER_DEPENDENCIES ? 2 : 0),
      14 * 60 + 15 + (RT_SUPPORTS_FEEDER_DEPENDENCIES ? 2 : 0));

  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(0, 1.0);

  data_departure_interchange data(
      true, ic_data.tail_node_departing_train_,
      *ic_data.arriving_route_edge_.to_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, dummy_arrival_distribution, dummy,
      dummy_node, context(get_schedule(), dummy, s_t_distributions));

  ASSERT_EQ(test_util::minutes_to_motis_time(12 * 60 + 45),
            data.scheduled_departure_time_);
  ASSERT_EQ(3, data.largest_delay());

  ASSERT_EQ(3, data.maximum_waiting_time_);
  ASSERT_EQ(test_util::minutes_to_motis_time(12 * 60 + 32),
            data.interchange_feeder_info_.scheduled_arrival_time_);
  ASSERT_EQ(&dummy_arrival_distribution,
            data.interchange_feeder_info_.arrival_distribution_);
  ASSERT_EQ(
      get_schedule()
          .stations_[ic_data.tail_node_departing_train_.station_node_->id_]
          ->transfer_time_,
      data.interchange_feeder_info_.transfer_time_);
  ASSERT_EQ(2, data.interchange_feeder_info_.transfer_time_);
  ASSERT_EQ(3, data.interchange_feeder_info_.waiting_time_);
  ASSERT_EQ((test_util::minutes_to_motis_time(12 * 60 + 45) + 3) - 2,
            data.interchange_feeder_info_.latest_feasible_arrival_);
}

TEST_F(reliability_realtime_data_interchange_walk, interchange_walk) {
  distributions_container::container dummy;
  distributions_container::container::node dummy_node;
  start_and_travel_test_distributions s_t_distributions({0.4, 0.4, 0.2});

  publish(realtime::get_delay_message(
      schedule3::FRANKFURT.eva_, schedule3::ICE_L_H, "", EventType_ARR,
      1443427800 /* 2015-09-28 10:10:00 GMT+2:00 */,
      1443427860 /* 2015-09-28 10:11:00 GMT */, schedule3::LANGEN.eva_,
      schedule3::ICE_L_H, 1443427200 /* 2015-09-28 10:00:00 GMT+2:00 */,
      ris::DelayType_Is));
  publish(make_no_msg("/ris/system_time_changed"));

  // arriving train ICE_L_H from Langen to Frankfurt
  // interchange at Frankfurt and walking to Messe
  // departing train S_M_W from Messe to West
  interchange_data_for_tests const ic_data(
      get_schedule(), schedule3::ICE_L_H, schedule3::S_M_W,
      schedule3::LANGEN.eva_, schedule3::FRANKFURT.eva_, schedule3::MESSE.eva_,
      schedule3::WEST.eva_, 10 * 60, 10 * 60 + 11, 10 * 60 + 20, 10 * 60 + 25);

  probability_distribution dummy_arrival_distribution;
  dummy_arrival_distribution.init_one_point(1, 1.0);

  data_departure_interchange_walk data(
      true, ic_data.tail_node_departing_train_,
      *ic_data.arriving_route_edge_.to_, ic_data.departing_light_conn_,
      ic_data.arriving_light_conn_, dummy_arrival_distribution, dummy,
      dummy_node, context(get_schedule(), dummy, s_t_distributions));

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
}  // namespace calc_departure_distribution
}  // namespace reliability
}  // namespace motis
