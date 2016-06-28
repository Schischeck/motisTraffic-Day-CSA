#include "gtest/gtest.h"

#include <map>

#include "motis/core/access/realtime_access.h"
#include "motis/core/access/trip_access.h"
#include "motis/test/motis_instance_test.h"
#include "motis/test/schedule/invalid_realtime.h"

using namespace motis;
using namespace motis::module;
using namespace motis::test;
using namespace motis::test::schedule;
using motis::test::schedule::invalid_realtime::dataset_opt;
using motis::test::schedule::invalid_realtime::get_trip_conflict_ris_message;
using motis::test::schedule::invalid_realtime::get_ts_conflict_ris_message;

struct rt_invalid_update_test : public motis_instance_test {
  struct stop_times {
    motis::time arr_, dep_;
  };

  using trip_event_info = std::map<std::string /* station id */, stop_times>;

  trip_event_info get_trip_event_info(trip const* trp) {
    trip_event_info trp_ev_info;
    for (auto const& trip_e : *trp->edges_) {
      auto const e = trip_e.get_edge();
      trp_ev_info[sched().stations_.at(e->from_->get_station()->id_)->eva_nr_]
          .dep_ = e->get_connection(trp->lcon_idx_)->d_time_;
      trp_ev_info[sched().stations_.at(e->to_->get_station()->id_)->eva_nr_]
          .arr_ = e->get_connection(trp->lcon_idx_)->a_time_;
    }
    return trp_ev_info;
  }

  rt_invalid_update_test()
      : motis::test::motis_instance_test(dataset_opt, {"rt"}) {}
};

TEST_F(rt_invalid_update_test, trip_conflict_test) {
  publish(get_trip_conflict_ris_message(sched()));
  publish(make_no_msg("/ris/system_time_changed"));

  auto ev1 =
      get_trip_event_info(get_trip(sched(), "0000001", 1, unix_time(1010),
                                   "0000005", unix_time(1400), "381"));
  EXPECT_EQ(motis_time(1010), ev1["0000001"].dep_);
  EXPECT_EQ(motis_time(1100), ev1["0000002"].arr_);
  EXPECT_EQ(motis_time(1301), ev1["0000002"].dep_);
  EXPECT_EQ(motis_time(1301), ev1["0000003"].arr_);
  EXPECT_EQ(motis_time(1301), ev1["0000003"].dep_);
  EXPECT_EQ(motis_time(1301), ev1["0000004"].arr_);
  EXPECT_EQ(motis_time(1310), ev1["0000004"].dep_);
  EXPECT_EQ(motis_time(1400), ev1["0000005"].arr_);

  auto ev2 =
      get_trip_event_info(get_trip(sched(), "0000001", 2, unix_time(1010),
                                   "0000004", unix_time(1300), "382"));
  EXPECT_EQ(motis_time(1010), ev2["0000001"].dep_);
  EXPECT_EQ(motis_time(1100), ev2["0000002"].arr_);
  EXPECT_EQ(motis_time(1110), ev2["0000002"].dep_);
  EXPECT_EQ(motis_time(1200), ev2["0000003"].arr_);
  EXPECT_EQ(motis_time(1210), ev2["0000003"].dep_);
  EXPECT_EQ(motis_time(1300), ev2["0000004"].arr_);
}

TEST_F(rt_invalid_update_test, ts_conflict_test) {
  publish(get_ts_conflict_ris_message(sched()));
  publish(make_no_msg("/ris/system_time_changed"));

  auto ev1 =
      get_trip_event_info(get_trip(sched(), "0000001", 1, unix_time(1010),
                                   "0000005", unix_time(1400), "381"));
  EXPECT_EQ(motis_time(1010), ev1["0000001"].dep_);
  EXPECT_EQ(motis_time(1100), ev1["0000002"].arr_);
  EXPECT_EQ(motis_time(1110), ev1["0000002"].dep_);
  EXPECT_EQ(motis_time(1200), ev1["0000003"].arr_);
  EXPECT_EQ(motis_time(1200), ev1["0000003"].dep_);
  EXPECT_EQ(motis_time(1200), ev1["0000004"].arr_);
  EXPECT_EQ(motis_time(1200), ev1["0000004"].dep_);
  EXPECT_EQ(motis_time(1400), ev1["0000005"].arr_);

  auto ev2 =
      get_trip_event_info(get_trip(sched(), "0000001", 2, unix_time(1010),
                                   "0000004", unix_time(1300), "382"));
  EXPECT_EQ(motis_time(1010), ev2["0000001"].dep_);
  EXPECT_EQ(motis_time(1100), ev2["0000002"].arr_);
  EXPECT_EQ(motis_time(1110), ev2["0000002"].dep_);
  EXPECT_EQ(motis_time(1200), ev2["0000003"].arr_);
  EXPECT_EQ(motis_time(1200), ev2["0000003"].dep_);
  EXPECT_EQ(motis_time(1200), ev2["0000004"].arr_);
}