#include "gtest/gtest.h"

#include "./include/helper.h"

#include "motis/core/access/time_access.h"
#include "motis/module/context/get_schedule.h"
#include "motis/module/message.h"
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

struct ris_forward_batched : public ::testing::Test {
  ris_forward_batched() {
    motis_ = launch_motis(kSchedulePath, kScheduleDate, {"ris"},
                          {"--ris.input_folder=NOT_EXISTING",
                           "--ris.database_file=" DATABASE_URL});
    motis_->run([this] {
      sched_ = &get_schedule();
      sched_begin_ = external_schedule_begin(*sched_);
      sched_end_ = external_schedule_end(*sched_);
    });

    subscribe(motis_, "/ris/messages", msg_sink(&msgs_));
    call(motis_, "/ris/init");

    sqlpp::sqlite3::connection_config conf;
    conf.path_to_database = DATABASE_URL;
    conf.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    db_ = std::make_unique<sqlpp::sqlite3::connection>(conf);
  }

  std::time_t unix_time(int hhmm, int day_idx = 0,
                        int timezone_offset = kDefaultTimezoneOffset) const {
    return motis::unix_time(*sched_, hhmm, day_idx, timezone_offset);
  }

  char const* extract_payload(RISBatch const* batch, int idx) const {
    return reinterpret_cast<char const*>(
        batch->messages()->Get(idx)->message()->Data());
  }

  bootstrap::motis_instance_ptr motis_;
  schedule const* sched_;
  std::time_t sched_begin_, sched_end_;

  std::vector<msg_ptr> msgs_;

  db_ptr db_;
  ris_database_util db_util_;
};

TEST_F(ris_forward_batched, no_msg) {
  auto end = unix_time(1200);

  motis_->run([&] { forward_batched(sched_begin_, sched_end_, end, db_); });
  ASSERT_EQ(0, msgs_.size());

  // outside schedule period
  db_util_  //
      .add_entry(unix_time(1200, -1), unix_time(1300, -1), unix_time(1100, -1))
      .add_entry(unix_time(1200, 2), unix_time(1300, 2), unix_time(1100, 2))
      .finish_packet(db_);
  motis_->run([&] {
    ASSERT_EQ(0, forward_batched(sched_begin_, sched_end_, end, db_));
  });
  ASSERT_EQ(0, msgs_.size());
}

TEST_F(ris_forward_batched, one_msg) {
  auto end = unix_time(1200);
  db_util_  //
      .add_entry(unix_time(1400), unix_time(1500), unix_time(1100), "before")
      .add_entry(unix_time(1400), unix_time(1500), unix_time(1300), "after")
      .finish_packet(db_);
  motis_->run([&] {
    ASSERT_EQ(unix_time(1100),
              forward_batched(sched_begin_, sched_end_, end, db_));
  });

  ASSERT_EQ(1, msgs_.size());
  auto batch = motis_content(RISBatch, msgs_[0]);
  ASSERT_EQ(1, batch->messages()->size());

  EXPECT_STREQ("before", extract_payload(batch, 0));
}

TEST_F(ris_forward_batched, one_batch) {
  auto t1 = unix_time(1200);
  db_util_  //
      .add_entry(unix_time(1400), unix_time(1500), unix_time(1001), "a")
      .add_entry(unix_time(1400), unix_time(1500), unix_time(1000), "b")
      .finish_packet(db_);
  motis_->run([&] {
    ASSERT_EQ(unix_time(1001),
              forward_batched(sched_begin_, sched_end_, t1, db_));
  });
  {
    ASSERT_EQ(1, msgs_.size());
    auto batch = motis_content(RISBatch, msgs_[0]);
    ASSERT_EQ(2, batch->messages()->size());
    msgs_.clear();
  }

  db_util_  //
      .add_entry(unix_time(1400), unix_time(1500), unix_time(1059) + 59, "c")
      .add_entry(unix_time(1400), unix_time(1500), unix_time(1100), "d")
      .add_entry(unix_time(1400), unix_time(1500), unix_time(1159) + 59, "e")
      .add_entry(unix_time(1400), unix_time(1500), unix_time(1200), "f")
      .add_entry(unix_time(1400), unix_time(1500), unix_time(1200) + 1, "g")
      .add_entry(unix_time(1400), unix_time(1500), unix_time(1300), "h")
      .add_entry(unix_time(1400), unix_time(1500), unix_time(1400), "i")
      .add_entry(unix_time(1400), unix_time(1500), unix_time(1400) + 1, "j")
      .finish_packet(db_);
  motis_->run([&] {
    ASSERT_EQ(unix_time(1200),
              forward_batched(sched_begin_, sched_end_, t1, db_));
  });
  {
    ASSERT_EQ(3, msgs_.size());
    auto batch1 = motis_content(RISBatch, msgs_[0]);
    ASSERT_EQ(3, batch1->messages()->size());
    EXPECT_STREQ("b", extract_payload(batch1, 0));
    EXPECT_STREQ("a", extract_payload(batch1, 1));
    EXPECT_STREQ("c", extract_payload(batch1, 2));

    auto batch2 = motis_content(RISBatch, msgs_[1]);
    ASSERT_EQ(2, batch2->messages()->size());
    EXPECT_STREQ("d", extract_payload(batch2, 0));
    EXPECT_STREQ("e", extract_payload(batch2, 1));

    auto batch3 = motis_content(RISBatch, msgs_[2]);
    ASSERT_EQ(1, batch3->messages()->size());
    EXPECT_STREQ("f", extract_payload(batch3, 0));
    msgs_.clear();
  }

  auto t2 = unix_time(1400);
  motis_->run([&] {
    ASSERT_EQ(unix_time(1400),
              forward_batched(sched_begin_, sched_end_, t1, t2, db_));
  });
  {
    ASSERT_EQ(2, msgs_.size());
    auto batch1 = motis_content(RISBatch, msgs_[0]);
    ASSERT_EQ(2, batch1->messages()->size());
    EXPECT_STREQ("g", extract_payload(batch1, 0));
    EXPECT_STREQ("h", extract_payload(batch1, 1));

    auto batch2 = motis_content(RISBatch, msgs_[1]);
    ASSERT_EQ(1, batch2->messages()->size());
    EXPECT_STREQ("i", extract_payload(batch2, 0));
    msgs_.clear();
  }
}

}  // detail
}  // ris
}  // motis
