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

constexpr auto RT_SUPPORTS_FEEDER_DEPENDENCIES = false;

using namespace module;

class reliability_realtime_update : public test_motis_setup {
public:
  reliability_realtime_update()
      : test_motis_setup(schedule_realtime_update::PATH,
                         schedule_realtime_update::DATE, true) {}
};

TEST_F(reliability_realtime_update, is_message) {
  auto const& n_darm = *graph_accessor::get_first_route_node(
      get_schedule(), schedule_realtime_update::ICE_D_L_F);
  auto const& lc_darm_langen = graph_accessor::get_departing_route_edge(n_darm)
                                   ->m_.route_edge_.conns_[0];
  {
    auto const& dist_dep_darm =
        reliability_context_->precomputed_distributions_.get_distribution(
            distributions_container::to_container_key(
                n_darm, lc_darm_langen, time_util::event_type::departure,
                get_schedule()));
    ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60), lc_darm_langen.d_time_);
    ASSERT_EQ(0, dist_dep_darm.first_minute());
  }
  publish(realtime::get_delay_message(
      schedule_realtime_update::DARMSTADT, schedule_realtime_update::ICE_D_L_F,
      "", ris::EventType_Departure,
      1445234400 /* 2015-10-19 08:00:00 GMT+2:00 */,
      1445235840 /* 2015-10-19 08:24:00 GMT+2:00 */,
      schedule_realtime_update::DARMSTADT, schedule_realtime_update::ICE_D_L_F,
      1445234400 /* 2015-10-19 08:00:00 GMT+2:00 */, ris::DelayType_Is));
  publish(make_no_msg("/ris/system_time_changed"));
  {
    ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 24),
              lc_darm_langen.d_time_);
    auto const& dist_dep_darm =
        reliability_context_->precomputed_distributions_.get_distribution(
            distributions_container::to_container_key(
                n_darm, lc_darm_langen, time_util::event_type::departure,
                get_schedule()));
    ASSERT_EQ(24, dist_dep_darm.first_minute());
    ASSERT_EQ(24, dist_dep_darm.last_minute());
    ASSERT_TRUE(equal(1.0, dist_dep_darm.probability_equal(24)));
  }
  auto const& n_langen = *graph_accessor::get_departing_route_edge(n_darm)->to_;
  {
    auto const& dist_arr_langen =
        reliability_context_->precomputed_distributions_.get_distribution(
            distributions_container::to_container_key(
                n_langen, lc_darm_langen, time_util::event_type::arrival,
                get_schedule()));
    ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 34),
              lc_darm_langen.a_time_);
    ASSERT_EQ(23, dist_arr_langen.first_minute());
    ASSERT_EQ(25, dist_arr_langen.last_minute());
    ASSERT_TRUE(equal(0.1, dist_arr_langen.probability_equal(23)));
    ASSERT_TRUE(equal(0.8, dist_arr_langen.probability_equal(24)));
    ASSERT_TRUE(equal(0.1, dist_arr_langen.probability_equal(25)));
  }
  auto const& lc_langen_ffm =
      graph_accessor::get_departing_route_edge(n_langen)->m_.route_edge_.conns_[0];
  {
    auto const& dist_dep_langen =
        reliability_context_->precomputed_distributions_.get_distribution(
            distributions_container::to_container_key(
                n_langen, lc_langen_ffm, time_util::event_type::departure,
                get_schedule()));
    ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 36),
              lc_langen_ffm.d_time_);
    ASSERT_EQ(22, dist_dep_langen.first_minute());
    ASSERT_EQ(24, dist_dep_langen.last_minute());
    ASSERT_TRUE(equal(0.1, dist_dep_langen.probability_equal(22)));
    ASSERT_TRUE(equal(0.8, dist_dep_langen.probability_equal(23)));
    ASSERT_TRUE(equal(0.1, dist_dep_langen.probability_equal(24)));
  }
  {
    auto const& n_ffm = *graph_accessor::get_departing_route_edge(n_langen)->to_;
    auto const& dist_arr_ffm =
        reliability_context_->precomputed_distributions_.get_distribution(
            distributions_container::to_container_key(
                n_ffm, lc_langen_ffm, time_util::event_type::arrival,
                get_schedule()));
    ASSERT_EQ(test_util::minutes_to_motis_time(8 * 60 + 41),
              lc_langen_ffm.a_time_);
    ASSERT_EQ(21, dist_arr_ffm.first_minute());
    ASSERT_EQ(25, dist_arr_ffm.last_minute());
    ASSERT_TRUE(equal(0.01, dist_arr_ffm.probability_equal(21)));
    ASSERT_TRUE(equal(0.16, dist_arr_ffm.probability_equal(22)));
    ASSERT_TRUE(equal(0.66, dist_arr_ffm.probability_equal(23)));
    ASSERT_TRUE(equal(0.16, dist_arr_ffm.probability_equal(24)));
    ASSERT_TRUE(equal(0.01, dist_arr_ffm.probability_equal(25)));
  }

  auto const& n_ffm = *graph_accessor::get_first_route_node(
      get_schedule(), schedule_realtime_update::ICE_F_H);
  auto const& lc_ffm_hanau =
      graph_accessor::get_departing_route_edge(n_ffm)->m_.route_edge_.conns_[0];
  {
    auto const& dist_dep_ffm =
        reliability_context_->precomputed_distributions_.get_distribution(
            distributions_container::to_container_key(
                n_ffm, lc_ffm_hanau, time_util::event_type::departure,
                get_schedule()));
    ASSERT_EQ(test_util::minutes_to_motis_time(
                  8 * 60 + 45 + (RT_SUPPORTS_FEEDER_DEPENDENCIES ? 1 : 0)),
              lc_ffm_hanau.d_time_);
    ASSERT_EQ(0, dist_dep_ffm.first_minute());
    ASSERT_EQ(3, dist_dep_ffm.last_minute());
    ASSERT_TRUE(equal(0.13599999999999987, dist_dep_ffm.probability_equal(0)));
    ASSERT_TRUE(equal(0.694, dist_dep_ffm.probability_equal(1)));
    ASSERT_TRUE(equal(0.16, dist_dep_ffm.probability_equal(2)));
    ASSERT_TRUE(equal(0.010000000000000009, dist_dep_ffm.probability_equal(3)));
  }
  {
    auto const& n_hanau = *graph_accessor::get_departing_route_edge(n_ffm)->to_;
    auto const& dist_arr_hanau =
        reliability_context_->precomputed_distributions_.get_distribution(
            distributions_container::to_container_key(
                n_hanau, lc_ffm_hanau, time_util::event_type::arrival,
                get_schedule()));
    ASSERT_EQ(test_util::minutes_to_motis_time(
                  9 * 60 + (RT_SUPPORTS_FEEDER_DEPENDENCIES ? 1 : 0)),
              lc_ffm_hanau.a_time_);
    ASSERT_EQ(-1, dist_arr_hanau.first_minute());
    ASSERT_EQ(4, dist_arr_hanau.last_minute());
    ASSERT_TRUE(
        equal(0.013599999999999987, dist_arr_hanau.probability_equal(-1)));
    ASSERT_TRUE(
        equal(0.17819999999999991, dist_arr_hanau.probability_equal(0)));
    ASSERT_TRUE(
        equal(0.58479999999999999, dist_arr_hanau.probability_equal(1)));
    ASSERT_TRUE(
        equal(0.19840000000000002, dist_arr_hanau.probability_equal(2)));
    ASSERT_TRUE(
        equal(0.024000000000000021, dist_arr_hanau.probability_equal(3)));
    ASSERT_TRUE(
        equal(0.0010000000000000009, dist_arr_hanau.probability_equal(4)));
  }
}
}  // namespace realtime
}  // namespace reliability
}  // namespace motis
