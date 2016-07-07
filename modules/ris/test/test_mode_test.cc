#include "gtest/gtest.h"

#include "motis/module/message.h"

#include "motis/test/motis_instance_test.h"
#include "motis/test/schedule/rename_at_first_stop.h"

using namespace motis;
using namespace motis::ris;
using namespace motis::module;
using namespace motis::test;
using namespace motis::test::schedule::rename_at_first_stop;

struct ris_test_mode : public motis_instance_test {
  ris_test_mode()
      : motis::test::motis_instance_test(dataset_opt, {"ris"},
                                         {"--ris.mode=test", kRisFolderArg}) {}
};

TEST_F(ris_test_mode, simple) {
  std::vector<msg_ptr> msgs;
  subscribe("/ris/messages", msg_sink(&msgs));

  std::vector<msg_ptr> time_changed;
  subscribe("/ris/system_time_changed", msg_sink(&time_changed));

  publish("/init");

  ASSERT_EQ(1, msgs.size());
  auto batch = motis_content(RISBatch, msgs[0]);
  ASSERT_EQ(2, batch->messages()->size());

  ASSERT_EQ(1, time_changed.size());
}
