#include "gtest/gtest.h"

#include "motis/reliability/computation/calc_arrival_distribution.h"
#include "motis/reliability/computation/calc_departure_distribution.h"
#include "motis/reliability/computation/data_arrival.h"
#include "motis/reliability/computation/data_departure.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/realtime/time_util.h"

#include "../include/message_builder.h"
#include "../include/schedules/schedule_realtime.h"
#include "../include/start_and_travel_test_distributions.h"
#include "../include/test_container.h"
#include "../include/test_schedule_setup.h"
#include "../include/test_util.h"

namespace motis {
namespace reliability {
using namespace time_util;
using namespace module;

class reliability_realtime_dist_data_test : public test_motis_setup {
public:
  reliability_realtime_dist_data_test()
      : test_motis_setup(schedule_realtime::PATH, schedule_realtime::DATE,
                         true) {}

  void test_departure_frankfurt(bool is_message, time is_time = 0) {
    auto const& route_node = *graph_accessor::get_first_route_node(
        get_schedule(), schedule_realtime::ICE_F_L_D);
    auto const& light_conn =
        graph_accessor::get_departing_route_edge(route_node)
            ->m_.route_edge_.conns_[0];

    ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 1), light_conn.d_time_);
    ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60),
              get_scheduled_event_time(route_node, light_conn, event_type::DEP,
                                       get_schedule()));

    distributions_container::container dummy;
    distributions_container::container::node dummy_node;
    start_and_travel_test_distributions s_t_distributions({0.8, 0.2});
    calc_departure_distribution::data_departure data(
        route_node, light_conn, true, dummy, dummy_node,
        context(get_schedule(), dummy, s_t_distributions));

    ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60),
              data.scheduled_departure_time_);
    ASSERT_EQ(1, data.largest_delay());
    auto const& start_distribution =
        *data.train_info_.first_departure_distribution_;
    ASSERT_TRUE(equal(start_distribution.sum(), 1.0));
    ASSERT_EQ(0, start_distribution.first_minute());
    ASSERT_EQ(1, start_distribution.last_minute());
    ASSERT_TRUE(equal(0.8, start_distribution.probability_equal(0)));
    ASSERT_TRUE(equal(0.2, start_distribution.probability_equal(1)));

    ASSERT_EQ(is_message, data.is_message_.received_);
    if (is_message) {
      ASSERT_EQ(is_time, data.is_message_.current_time_);

      probability_distribution pd;
      calc_departure_distribution::compute_departure_distribution(data, pd);
      int const delay =
          data.is_message_.current_time_ - data.scheduled_departure_time_;
      ASSERT_EQ(delay, pd.first_minute());
      ASSERT_EQ(delay, pd.last_minute());
      ASSERT_TRUE(equal(1.0, pd.probability_equal(delay)));
    }
  }

  void test_arrival_langen(bool is_message, time is_time = 0) {
    auto const& departing_route_node = *graph_accessor::get_first_route_node(
        get_schedule(), schedule_realtime::ICE_F_L_D);
    auto const& route_edge =
        *graph_accessor::get_departing_route_edge(departing_route_node);
    auto const& arriving_route_node = *route_edge.to_;
    auto const& light_conn = route_edge.m_.route_edge_.conns_[0];

    ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 5),
              get_scheduled_event_time(arriving_route_node, light_conn,
                                       event_type::ARR, get_schedule()));
    ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 6), light_conn.a_time_);

    start_and_travel_test_distributions s_t_distributions({0.1, 0.8, 0.1}, -1);
    probability_distribution dep_dist;
    dep_dist.init({0.8, 0.2}, 0);

    calc_arrival_distribution::data_arrival data(
        departing_route_node, arriving_route_node, light_conn, dep_dist,
        get_schedule(), s_t_distributions);

    ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60),
              data.departure_info_.scheduled_departure_time_);
    ASSERT_EQ(&dep_dist, &data.departure_info_.distribution_);

    ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 5),
              data.scheduled_arrival_time_);

    ASSERT_TRUE(data.travel_distributions_.size() == 2);
    ASSERT_TRUE(&data.travel_distributions_[0].get() ==
                &s_t_distributions.travel_distribution_);
    ASSERT_TRUE(&data.travel_distributions_[1].get() ==
                &s_t_distributions.travel_distribution_);

    ASSERT_EQ(-1, data.left_bound_);
    ASSERT_EQ(2, data.right_bound_);

    ASSERT_EQ(is_message, data.is_message_.received_);
    if (is_message) {
      ASSERT_EQ(is_time, data.is_message_.current_time_);

      probability_distribution pd;
      calc_arrival_distribution::compute_arrival_distribution(data, pd);
      int const delay =
          data.is_message_.current_time_ - data.scheduled_arrival_time_;
      ASSERT_EQ(delay, pd.first_minute());
      ASSERT_EQ(delay, pd.last_minute());
      ASSERT_TRUE(equal(1.0, pd.probability_equal(delay)));
    }
  }

  void test_departure_langen(bool is_message, time is_time = 0) {
    auto const& route_node =
        *graph_accessor::get_departing_route_edge(
             *graph_accessor::get_first_route_node(
                 get_schedule(), schedule_realtime::ICE_F_L_D))
             ->to_;
    auto const& light_conn =
        graph_accessor::get_departing_route_edge(route_node)
            ->m_.route_edge_.conns_[0];

    ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 8), light_conn.d_time_);
    ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 7),
              get_scheduled_event_time(route_node, light_conn, event_type::DEP,
                                       get_schedule()));

    auto const& arriving_light_conn =
        graph_accessor::get_arriving_route_edge(route_node)
            ->m_.route_edge_.conns_[0];
    ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 6),
              arriving_light_conn.a_time_);
    ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 5),
              get_scheduled_event_time(route_node, arriving_light_conn,
                                       event_type::ARR, get_schedule()));

    distributions_container::container dummy;
    distributions_container::container::node dummy_node;
    start_and_travel_test_distributions s_t_distributions({0.8, 0.2});
    probability_distribution arrival_dist;
    arrival_dist.init({0.9, 0.1}, 0);
    distributions_container::single_distribution_container train_dist(
        arrival_dist);
    calc_departure_distribution::data_departure data(
        route_node, light_conn, false, train_dist, dummy_node,
        context(get_schedule(), dummy, s_t_distributions));

    ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 7),
              data.scheduled_departure_time_);
    ASSERT_EQ(1, data.largest_delay());
    ASSERT_EQ(&arrival_dist,
              data.train_info_.preceding_arrival_info_.arrival_distribution_);
    ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 5),
              data.train_info_.preceding_arrival_info_.scheduled_arrival_time_);

    ASSERT_EQ(is_message, data.is_message_.received_);
    if (is_message) {
      ASSERT_EQ(is_time, data.is_message_.current_time_);

      probability_distribution pd;
      calc_departure_distribution::compute_departure_distribution(data, pd);
      int const delay =
          data.is_message_.current_time_ - data.scheduled_departure_time_;
      ASSERT_EQ(delay, pd.first_minute());
      ASSERT_EQ(delay, pd.last_minute());
      ASSERT_TRUE(equal(1.0, pd.probability_equal(delay)));
    }
  }
};

TEST_F(reliability_realtime_dist_data_test, get_scheduled_event_time) {
  auto const& first_route_node = *graph_accessor::get_first_route_node(
      get_schedule(), schedule_realtime::ICE_F_L_D);
  auto const& first_lc =
      graph_accessor::get_departing_route_edge(first_route_node)
          ->m_.route_edge_.conns_[0];

  ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60), first_lc.d_time_);
  ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60),
            get_scheduled_event_time(first_route_node, first_lc,
                                     event_type::DEP, get_schedule()));

  publish(realtime::get_delay_message(
      schedule_realtime::FRANKFURT, schedule_realtime::ICE_F_L_D, "",
      EventType_DEP, 1445234400 /* 2015-10-19 08:00:00 GMT+2:00 */,
      1445234460 /* 2015-10-19 08:01:00 GMT+2:00 */,
      schedule_realtime::FRANKFURT, schedule_realtime::ICE_F_L_D,
      1445234400 /* 2015-10-19 08:00:00 GMT+2:00 */, ris::DelayType_Is));
  publish(make_no_msg("/ris/system_time_changed"));
  ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 1), first_lc.d_time_);
  ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60),
            get_scheduled_event_time(first_route_node, first_lc,
                                     event_type::DEP, get_schedule()));
}

TEST_F(reliability_realtime_dist_data_test,
       data_departure_forecast_arrival_propagation) {
  publish(realtime::get_delay_message(
      schedule_realtime::FRANKFURT, schedule_realtime::ICE_F_L_D, "",
      EventType_DEP, 1445234400 /* 2015-10-19 08:00:00 GMT+2:00 */,
      1445234460 /* 2015-10-19 08:01:00 GMT+2:00 */,
      schedule_realtime::FRANKFURT, schedule_realtime::ICE_F_L_D,
      1445234400 /* 2015-10-19 08:00:00 GMT+2:00 */, ris::DelayType_Forecast));
  publish(make_no_msg("/ris/system_time_changed"));

  test_departure_frankfurt(false); /* test departure forecast */
  test_arrival_langen(false); /* test arrival propagation */
}

TEST_F(reliability_realtime_dist_data_test, data_arrival_forecast) {
  publish(realtime::get_delay_message(
      schedule_realtime::LANGEN, schedule_realtime::ICE_F_L_D, "",
      EventType_ARR, 1445234700 /* 2015-10-19 08:05:00 GMT+2:00 */,
      1445234760 /* 2015-10-19 08:06:00 GMT+2:00 */,
      schedule_realtime::FRANKFURT, schedule_realtime::ICE_F_L_D,
      1445234400 /* 2015-10-19 08:00:00 GMT+2:00 */, ris::DelayType_Forecast));
  publish(make_no_msg("/ris/system_time_changed"));
  test_arrival_langen(false);
  test_departure_langen(false);
}

TEST_F(reliability_realtime_dist_data_test,
       data_departure_is_arrival_propagation) {
  publish(realtime::get_delay_message(
      schedule_realtime::FRANKFURT, schedule_realtime::ICE_F_L_D, "",
      EventType_DEP, 1445234400 /* 2015-10-19 08:00:00 GMT+2:00 */,
      1445234460 /* 2015-10-19 08:01:00 GMT+2:00 */,
      schedule_realtime::FRANKFURT, schedule_realtime::ICE_F_L_D,
      1445234400 /* 2015-10-19 08:00:00 GMT+2:00 */, ris::DelayType_Is));
  publish(make_no_msg("/ris/system_time_changed"));

  test_departure_frankfurt(true, test_util::minutes_to_motis_time(
                                     8 * 60 + 1)); /* test departure forecast */
  test_arrival_langen(false); /* test arrival propagation */
}

TEST_F(reliability_realtime_dist_data_test, data_arrival_is) {
  auto const& route_edge = *graph_accessor::get_departing_route_edge(
      *graph_accessor::get_first_route_node(get_schedule(),
                                            schedule_realtime::ICE_F_L_D));
  auto const& route_node = *route_edge.to_;
  auto const& first_lc = route_edge.m_.route_edge_.conns_[0];

  ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 5), first_lc.a_time_);
  ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 5),
            get_scheduled_event_time(route_node, first_lc, event_type::ARR,
                                     get_schedule()));

  publish(realtime::get_delay_message(
      schedule_realtime::LANGEN, schedule_realtime::ICE_F_L_D, "",
      EventType_ARR, 1445234700 /* 2015-10-19 08:05:00 GMT+2:00 */,
      1445234760 /* 2015-10-19 08:06:00 GMT+2:00 */,
      schedule_realtime::FRANKFURT, schedule_realtime::ICE_F_L_D,
      1445234400 /* 2015-10-19 08:00:00 GMT+2:00 */, ris::DelayType_Is));
  publish(make_no_msg("/ris/system_time_changed"));
  test_arrival_langen(true, test_util::minutes_to_motis_time(8 * 60 + 6));
  test_departure_langen(false);
}

TEST_F(reliability_realtime_dist_data_test, data_feeder_is) {
  publish(realtime::get_delay_message(
      schedule_realtime::LANGEN, schedule_realtime::ICE_F_L_D, "",
      EventType_ARR, 1445234700 /* 2015-10-19 08:05:00 GMT+2:00 */,
      1445234760 /* 2015-10-19 08:06:00 GMT+2:00 */,
      schedule_realtime::FRANKFURT, schedule_realtime::ICE_F_L_D,
      1445234400 /* 2015-10-19 08:00:00 GMT+2:00 */, ris::DelayType_Is));
  publish(realtime::get_delay_message(
      schedule_realtime::LANGEN, schedule_realtime::ICE_L_H, "", EventType_DEP,
      1445235000 /* 2015-10-19 08:10:00 GMT+2:00 */,
      1445235120 /* 2015-10-19 08:12:00 GMT+2:00 */, schedule_realtime::LANGEN,
      schedule_realtime::ICE_L_H, 1445235000 /* 2015-10-19 08:10:00 GMT+2:00 */,
      ris::DelayType_Forecast));
  publish(make_no_msg("/ris/system_time_changed"));

  auto const& route_node = *graph_accessor::get_first_route_node(
      get_schedule(), schedule_realtime::ICE_L_H);
  auto const& light_conn = graph_accessor::get_departing_route_edge(route_node)
                               ->m_.route_edge_.conns_[0];

  ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 12), light_conn.d_time_);
  ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 10),
            get_scheduled_event_time(route_node, light_conn, event_type::DEP,
                                     get_schedule()));

  distributions_container::container dummy;
  probability_distribution feeder_dist;
  feeder_dist.init({0.9, 0.1}, 0);
  distributions_container::container feeder_dist_container;
  auto const& distribution_node = init_feeders_and_get_distribution_node(
      feeder_dist_container, route_node, light_conn, {0.9, 0.1}, 0,
      get_schedule());

  start_and_travel_test_distributions s_t_distributions({0.8, 0.2});
  calc_departure_distribution::data_departure data(
      route_node, light_conn, true, dummy, distribution_node,
      context(get_schedule(), feeder_dist_container, s_t_distributions));

  ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 10),
            data.scheduled_departure_time_);
  ASSERT_EQ(1, data.feeders_.size());
  auto const& feeder = data.feeders_.front();
  ASSERT_EQ(feeder_dist, feeder.distribution_);
  ASSERT_EQ(test_util::minutes_to_motis_time((8 * 60 + 10 + 3) - 5),
            feeder.latest_feasible_arrival_);
  ASSERT_EQ(5, feeder.transfer_time_);
  ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 5),
            feeder.scheduled_arrival_time_);
}

}  // namespace reliability
}  // namespace motis
