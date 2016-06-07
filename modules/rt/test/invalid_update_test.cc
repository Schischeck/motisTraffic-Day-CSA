#include "gtest/gtest.h"

#include "motis/core/access/trip_access.h"
#include "motis/test/motis_instance_test.h"
#include "motis/test/schedule/invalid_realtime.h"

using namespace motis;
using namespace motis::module;
using namespace motis::test;
using namespace motis::test::schedule;
using motis::test::schedule::invalid_realtime::dataset_opt;
using motis::test::schedule::invalid_realtime::get_ris_message;

struct rt_invalid_update_test : public motis_instance_test {
  rt_invalid_update_test()
      : motis::test::motis_instance_test(dataset_opt, {"rt"}) {}
};

TEST_F(rt_invalid_update_test, valid_graph) {
  publish(get_ris_message(sched()));
  publish(make_no_msg("/ris/system_time_changed"));

  auto trp1 = get_trip(sched(), "0000001", 1, unix_time(1010), "0000005",
                       unix_time(1400), false, "381");

  auto lcon1 =
      trp1->edges_->at(0).get_edge()->m_.route_edge_.conns_[trp1->lcon_idx_];
  EXPECT_FALSE(lcon1.valid_);

  auto trp2 = get_trip(sched(), "0000001", 2, unix_time(1015), "0000005",
                       unix_time(1405), false, "381");
  auto lcon2 =
      trp1->edges_->at(0).get_edge()->m_.route_edge_.conns_[trp2->lcon_idx_];
  EXPECT_TRUE(lcon2.valid_);

  auto trp3 = get_trip(sched(), "0000001", 3, unix_time(1020), "0000005",
                       unix_time(1410), false, "381");
  auto lcon3 =
      trp1->edges_->at(0).get_edge()->m_.route_edge_.conns_[trp3->lcon_idx_];
  EXPECT_TRUE(lcon3.valid_);
}
