#include "gtest/gtest.h"

#include "motis/module/message.h"
#include "motis/test/motis_instance_test.h"
#include "motis/test/schedule/simple_realtime.h"

using namespace motis;
using namespace motis::module;
using namespace motis::test;
using motis::test::schedule::simple_realtime::dataset_opt;
using motis::test::schedule::simple_realtime::get_ris_message;

struct rt_test : public motis_instance_test {
  rt_test() : motis_instance_test(dataset_opt, {"rt"}) {}
};

TEST_F(rt_test, simple) { publish(get_ris_message()); }
