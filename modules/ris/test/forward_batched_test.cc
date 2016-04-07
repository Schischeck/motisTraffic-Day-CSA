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
namespace detail {

TEST(ris_forward_batched, simple) {
  auto motis = launch_motis(kSchedulePath, kScheduleDate, {"ris"},
                            {"--ris.mode=test", kRisFolderArg});
  msg_ptr msg;
  motis->subscribe("/ris/messages", [&msg](msg_ptr const& m) { msg = m; });

  motis->call("/ris/init");

  ASSERT_TRUE(msg);
  ASSERT_EQ(MsgContent_RISBatch, msg->content_type());

  auto batch = msg->content<RISBatch const*>();
  ASSERT_EQ(2, batch->messages()->size());
}

}  // detail
}  // ris
}  // motis
