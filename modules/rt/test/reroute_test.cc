#include "gtest/gtest.h"

#include <map>

#include "motis/core/access/realtime_access.h"
#include "motis/core/access/trip_access.h"
#include "motis/test/motis_instance_test.h"
#include "motis/test/schedule/invalid_realtime.h"

#include "./get_trip_event_info.h"

using namespace motis;
using namespace motis::module;
using namespace motis::rt;
using namespace motis::test;
using namespace motis::test::schedule;
using motis::test::schedule::invalid_realtime::dataset_opt;
using motis::test::schedule::invalid_realtime::get_reroute_ris_message;

struct rt_reroute_test : public motis_instance_test {
  rt_reroute_test() : motis::test::motis_instance_test(dataset_opt, {"rt"}) {}

  void SetUp() override {
    publish(get_reroute_ris_message(sched()));
    publish(make_no_msg("/ris/system_time_changed"));
  }
};

TEST_F(rt_reroute_test, reroute_with_delay_times) {
  auto evs = get_trip_event_info(
      sched(), get_trip(sched(), "0000001", 1, unix_time(1010), "0000005",
                        unix_time(1400), "381"));
  EXPECT_EQ(motis_time(910), evs["0000005"].dep_);
  EXPECT_EQ(motis_time(1105), evs["0000002"].arr_);
  EXPECT_EQ(motis_time(1112), evs["0000002"].dep_);
  EXPECT_EQ(motis_time(1305), evs["0000004"].arr_);
  EXPECT_EQ(motis_time(1312), evs["0000004"].dep_);
  EXPECT_EQ(motis_time(1500), evs["0000001"].arr_);
}

TEST_F(rt_reroute_test, reroute_with_delay_in_out) {
  auto ev1 = get_trip_event_info(
      sched(), get_trip(sched(), "0000001", 1, unix_time(1010), "0000005",
                        unix_time(1400), "381"));
  EXPECT_EQ(in_out_allowed(true, true), ev1["0000005"].in_out_);
  EXPECT_EQ(in_out_allowed(true, true), ev1["0000002"].in_out_);
  EXPECT_EQ(in_out_allowed(false, false), ev1["0000004"].in_out_);
  EXPECT_EQ(in_out_allowed(true, true), ev1["0000001"].in_out_);
}
