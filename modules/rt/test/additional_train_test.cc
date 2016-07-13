#include "gtest/gtest.h"

#include "motis/core/access/trip_access.h"
#include "motis/rt/separate_trip.h"
#include "motis/test/motis_instance_test.h"
#include "motis/test/schedule/invalid_realtime.h"

using namespace motis;
using namespace motis::rt;
using namespace motis::test;
using namespace motis::module;
using motis::test::schedule::invalid_realtime::dataset_opt;
using motis::test::schedule::invalid_realtime::get_additional_ris_message;

struct rt_additional_service_test : public motis_instance_test {
  rt_additional_service_test()
      : motis::test::motis_instance_test(dataset_opt, {"rt"}) {}
};

TEST_F(rt_additional_service_test, simple) {
  publish(get_additional_ris_message(sched()));
  publish(make_no_msg("/ris/system_time_changed"));
}
