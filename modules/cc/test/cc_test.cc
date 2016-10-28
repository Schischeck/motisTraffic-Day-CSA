#include "gtest/gtest.h"

#include "motis/core/access/time_access.h"
#include "motis/core/access/trip_access.h"
#include "motis/rt/separate_trip.h"
#include "motis/test/motis_instance_test.h"
#include "motis/test/schedule/invalid_realtime.h"

using namespace motis;
using namespace motis::test;
using namespace motis::module;
using motis::test::schedule::invalid_realtime::dataset_opt;

struct cc_check_artificial_connection_test : public motis_instance_test {
  cc_check_artificial_connection_test()
      : motis::test::motis_instance_test(dataset_opt, {"cc"}) {}
};

TEST_F(cc_check_artificial_connection_test, simple) {
  // TODO(felix) check whether routed connection with at least one interchange
  // and at least one walk (start + end) is considered okay
}
