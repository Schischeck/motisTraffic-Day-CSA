#include "gtest/gtest.h"

#include "motis/module/message.h"

#include "motis/test/motis_instance_helper.h"
#include "motis/test/schedule/rename_at_first_stop.h"

using namespace motis::ris;
using namespace motis::module;
using namespace motis::test;
using namespace motis::test::schedule::rename_at_first_stop;

namespace motis {
namespace ris {
namespace mode {

TEST(ris_test_mode, simple) {
  auto motis = launch_motis(kSchedulePath, kScheduleDate, {"ris"},
                            {"--ris.mode=test", kRisFolderArg});
  std::vector<msg_ptr> msgs;
  subscribe(motis, "/ris/messages", msg_sink(&msgs));
  call(motis, "/ris/init");

  ASSERT_EQ(1, msgs.size());
  auto batch = motis_content(RISBatch, msgs[0]);
  ASSERT_EQ(2, batch->messages()->size());
}

}  // mode
}  // ris
}  // motis
