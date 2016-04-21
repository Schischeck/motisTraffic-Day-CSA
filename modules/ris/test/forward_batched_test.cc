#include "gtest/gtest.h"

#include "motis/core/access/time_access.h"
#include "motis/module/message.h"
#include "motis/module/get_schedule.h"
#include "motis/ris/detail/forward_batched.h"

#include "motis/test/motis_instance_helper.h"
#include "motis/test/schedule/rename_at_first_stop.h"

using namespace motis::ris;
using namespace motis::module;
using namespace motis::test;
using namespace motis::test::schedule::rename_at_first_stop;

namespace motis {
namespace ris {
namespace detail {

#define DATABASE_URL "file:ris_forward_batched?mode=memory&cache=shared"

class ris_forward_batched : public ::testing::Test {
protected:
  ris_forward_batched()
      : motis_(launch_motis(kSchedulePath, kScheduleDate, {"ris"},
                            {"--ris.input_folder=NOT_EXISTING",
                             "--ris.database_file=" DATABASE_URL})) {}
public:
  bootstrap::motis_instance_ptr motis_;
};

TEST_F(ris_forward_batched, forwarder) {
  std::vector<msg_ptr> msgs;
  subscribe(motis_, "/ris/messages", msg_sink(&msgs));
  call(motis_, "/ris/init");

  motis_->run([&] {
    sqlpp::sqlite3::connection_config conf;
    conf.path_to_database = DATABASE_URL;
    conf.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    auto db = std::make_unique<sqlpp::sqlite3::connection>(conf);

    auto const& sched = get_schedule();
    auto schedule_begin = external_schedule_begin(sched);
    auto schedule_end = external_schedule_end(sched);

    forward_batched(schedule_begin, schedule_end, 0, 10, db);

    ASSERT_EQ(0, msgs.size());
  });
}

}  // detail
}  // ris
}  // motis
