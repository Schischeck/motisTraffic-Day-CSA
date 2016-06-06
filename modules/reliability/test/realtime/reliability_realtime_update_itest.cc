#include "gtest/gtest.h"

#include "motis/reliability/distributions/distributions_container.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/realtime/realtime_update.h"
#include "motis/reliability/realtime/time_util.h"

#include "../include/message_builder.h"
#include "../include/schedules/schedule_realtime_update.h"
#include "../include/start_and_travel_test_distributions.h"
#include "../include/test_schedule_setup.h"
#include "../include/test_util.h"

namespace motis {
namespace reliability {
namespace realtime {

class reliability_realtime_update : public test_motis_setup {
public:
  reliability_realtime_update()
      : test_motis_setup(schedule_realtime_update::PATH,
                         schedule_realtime_update::DATE, true) {}
};

TEST_F(reliability_realtime_update, is_message) {
  auto const& n_D = *graph_accessor::get_first_route_node(
      get_schedule(), schedule_realtime_update::ICE_D_L_F);
  auto const& lc_D_L =
      graph_accessor::get_departing_route_edge(n_D)->m_.route_edge_.conns_[0];
  {
    auto const& dist_dep_D =
        reliability_context_->precomputed_distributions_.get_distribution(
            distributions_container::to_container_key(
                n_D, lc_D_L, time_util::event_type::departure, get_schedule()));
    ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60), lc_D_L.d_time_);
    ASSERT_EQ(0, dist_dep_D.first_minute());
  }
  publish(realtime::get_delay_message(
      schedule_realtime_update::DARMSTADT, schedule_realtime_update::ICE_D_L_F,
      "", ris::EventType_Departure, 1445241600 /* 2015-10-19 08:00:00 GMT */,
      1445243040 /* 2015-10-19 08:24:00 GMT */, ris::DelayType_Is));
  {
    auto const& dist_dep_D =
        reliability_context_->precomputed_distributions_.get_distribution(
            distributions_container::to_container_key(
                n_D, lc_D_L, time_util::event_type::departure, get_schedule()));
    ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 24), lc_D_L.d_time_);
    ASSERT_EQ(24, dist_dep_D.first_minute());
    ASSERT_EQ(24, dist_dep_D.last_minute());
    ASSERT_TRUE(equal(1.0, dist_dep_D.probability_equal(24)));
  }
  auto const& n_L = *graph_accessor::get_departing_route_edge(n_D)->to_;
  {
    auto const& dist_arr_L =
        reliability_context_->precomputed_distributions_.get_distribution(
            distributions_container::to_container_key(
                n_L, lc_D_L, time_util::event_type::arrival, get_schedule()));
    ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 34), lc_D_L.a_time_);
    ASSERT_EQ(23, dist_arr_L.first_minute());
    ASSERT_EQ(25, dist_arr_L.last_minute());
    ASSERT_TRUE(equal(0.1, dist_arr_L.probability_equal(23)));
    ASSERT_TRUE(equal(0.8, dist_arr_L.probability_equal(24)));
    ASSERT_TRUE(equal(0.1, dist_arr_L.probability_equal(25)));
  }
  auto const& lc_L_F =
      graph_accessor::get_departing_route_edge(n_L)->m_.route_edge_.conns_[0];
  {
    auto const& dist_dep_L =
        reliability_context_->precomputed_distributions_.get_distribution(
            distributions_container::to_container_key(
                n_L, lc_L_F, time_util::event_type::departure, get_schedule()));
    ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 36), lc_L_F.d_time_);
    ASSERT_EQ(22, dist_dep_L.first_minute());
    ASSERT_EQ(24, dist_dep_L.last_minute());
    ASSERT_TRUE(equal(0.1, dist_dep_L.probability_equal(22)));
    ASSERT_TRUE(equal(0.8, dist_dep_L.probability_equal(23)));
    ASSERT_TRUE(equal(0.1, dist_dep_L.probability_equal(24)));
  }
  {
    auto const& n_F = *graph_accessor::get_departing_route_edge(n_L)->to_;
    auto const& dist_arr_F =
        reliability_context_->precomputed_distributions_.get_distribution(
            distributions_container::to_container_key(
                n_F, lc_L_F, time_util::event_type::arrival, get_schedule()));
    ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 41), lc_L_F.a_time_);
    ASSERT_EQ(21, dist_arr_F.first_minute());
    ASSERT_EQ(25, dist_arr_F.last_minute());
    ASSERT_TRUE(equal(0.01, dist_arr_F.probability_equal(21)));
    ASSERT_TRUE(equal(0.16, dist_arr_F.probability_equal(22)));
    ASSERT_TRUE(equal(0.66, dist_arr_F.probability_equal(23)));
    ASSERT_TRUE(equal(0.16, dist_arr_F.probability_equal(24)));
    ASSERT_TRUE(equal(0.01, dist_arr_F.probability_equal(25)));
  }

  auto const& n_F = *graph_accessor::get_first_route_node(
      get_schedule(), schedule_realtime_update::ICE_F_H);
  auto const& lc_F_H =
      graph_accessor::get_departing_route_edge(n_F)->m_.route_edge_.conns_[0];
  {
    auto const& dist_dep_F =
        reliability_context_->precomputed_distributions_.get_distribution(
            distributions_container::to_container_key(
                n_F, lc_F_H, time_util::event_type::departure, get_schedule()));
    ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 46), lc_F_H.d_time_);
    ASSERT_EQ(0, dist_dep_F.first_minute());
    ASSERT_EQ(3, dist_dep_F.last_minute());
    ASSERT_TRUE(equal(0.13599999999999987, dist_dep_F.probability_equal(0)));
    ASSERT_TRUE(equal(0.694, dist_dep_F.probability_equal(1)));
    ASSERT_TRUE(equal(0.16, dist_dep_F.probability_equal(2)));
    ASSERT_TRUE(equal(0.010000000000000009, dist_dep_F.probability_equal(3)));
  }
  {
    auto const& n_H = *graph_accessor::get_departing_route_edge(n_F)->to_;
    auto const& dist_arr_H =
        reliability_context_->precomputed_distributions_.get_distribution(
            distributions_container::to_container_key(
                n_H, lc_F_H, time_util::event_type::arrival, get_schedule()));
    ASSERT_EQ(test_util::minutes_to_motis_time(9 * 60 + 1), lc_F_H.a_time_);
    ASSERT_EQ(-1, dist_arr_H.first_minute());
    ASSERT_EQ(4, dist_arr_H.last_minute());
    ASSERT_TRUE(equal(0.013599999999999987, dist_arr_H.probability_equal(-1)));
    ASSERT_TRUE(equal(0.17819999999999991, dist_arr_H.probability_equal(0)));
    ASSERT_TRUE(equal(0.58479999999999999, dist_arr_H.probability_equal(1)));
    ASSERT_TRUE(equal(0.19840000000000002, dist_arr_H.probability_equal(2)));
    ASSERT_TRUE(equal(0.024000000000000021, dist_arr_H.probability_equal(3)));
    ASSERT_TRUE(equal(0.0010000000000000009, dist_arr_H.probability_equal(4)));
  }
}
}
}
}
