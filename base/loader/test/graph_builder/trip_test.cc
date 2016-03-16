#include "gtest/gtest.h"

#include "./graph_builder_test.h"

#include "motis/core/common/date_util.h"
#include "motis/core/access/trip_access.h"
#include "motis/core/access/trip_iterator.h"
#include "motis/core/access/trip_section.h"
#include "motis/core/access/trip_stop.h"

using namespace motis::access;

namespace motis {
namespace loader {

struct time_helper {
  time_helper(std::time_t const schedule_begin, int const timezone_offset = -60)
      : schedule_begin_(schedule_begin), timezone_offset_(timezone_offset) {}

  std::time_t unix(int h, int m = 0) {
    return motis_to_unixtime(schedule_begin_, motis(h, m));
  }

  motis::time motis(int h, int m = 0) {
    return SCHEDULE_OFFSET_MINUTES + h * 60 + m + timezone_offset_;
  }

  std::time_t schedule_begin_;
  int timezone_offset_;  // in minutes
};

class loader_trip : public loader_graph_builder_test {
public:
  loader_trip()
      : loader_graph_builder_test(
            "trip", to_unix_time(2015, 10, 25),
            to_unix_time(2015, 10, 25) + 2 * MINUTES_A_DAY * 60) {}
};

TEST_F(loader_trip, none) {
  time_helper t(sched_->schedule_begin_);

  ASSERT_ANY_THROW(get_trip(*sched_, "1234567", 31337, t.unix(1), "7654321",
                            t.unix(2), true, ""));
}

TEST_F(loader_trip, simple) {
  time_helper t(sched_->schedule_begin_);
  auto trp = get_trip(*sched_, "0000001", 1, t.unix(10), "0000003", t.unix(12),
                      false, "");
  ASSERT_NE(nullptr, trp);

  auto const& primary = trp->id.primary;
  auto const& secondary = trp->id.secondary;

  EXPECT_EQ("0000001", sched_->stations[primary.station_id]->eva_nr);
  EXPECT_EQ(1, primary.train_nr);
  EXPECT_EQ(t.motis(10), primary.time);

  EXPECT_EQ("", secondary.line_id);
  EXPECT_EQ("0000003", sched_->stations[secondary.target_station_id]->eva_nr);
  EXPECT_EQ(t.motis(12), secondary.target_time);
  EXPECT_EQ(false, secondary.is_arrival);

  ASSERT_EQ(2, trp->edges->size());
  for (auto const& sec : sections(trp)) {
    auto const& lcon = sec.lcon();
    auto const& info = sec.info(*sched_);
    auto const& from = sec.from_station(*sched_);
    auto const& to = sec.to_station(*sched_);

    switch (sec.index()) {
      case 0:
        EXPECT_EQ(t.motis(10), lcon.d_time);
        EXPECT_EQ(t.motis(11), lcon.a_time);
        EXPECT_EQ("0000001", from.eva_nr);
        EXPECT_EQ("0000002", to.eva_nr);
        EXPECT_EQ(1, info.train_nr);
        break;

      case 1:
        EXPECT_EQ(t.motis(11), lcon.d_time);
        EXPECT_EQ(t.motis(12), lcon.a_time);
        EXPECT_EQ("0000002", from.eva_nr);
        EXPECT_EQ("0000003", to.eva_nr);
        EXPECT_EQ(1, info.train_nr);
        break;

      default: FAIL() << "section index out of bounds";
    }
  }

  for (auto const& stop : stops(trp)) {
    auto const& station = stop.get_station(*sched_);
    switch (stop.index()) {
      case 0:
        EXPECT_EQ("0000001", station.eva_nr);
        ASSERT_FALSE(stop.has_arrival());
        ASSERT_TRUE(stop.has_departure());
        EXPECT_EQ(t.motis(10), stop.dep_lcon().d_time);

        break;

      case 1:
        EXPECT_EQ("0000002", station.eva_nr);
        ASSERT_TRUE(stop.has_arrival());
        ASSERT_TRUE(stop.has_departure());
        EXPECT_EQ(t.motis(11), stop.arr_lcon().a_time);
        EXPECT_EQ(t.motis(11), stop.dep_lcon().d_time);
        break;

      case 2:
        EXPECT_EQ("0000003", station.eva_nr);
        ASSERT_TRUE(stop.has_arrival());
        ASSERT_FALSE(stop.has_departure());
        EXPECT_EQ(t.motis(12), stop.arr_lcon().a_time);
        break;

      default: FAIL() << "stop index out of bounds";
    }
  }
}

TEST_F(loader_trip, collision) {
  time_helper t(sched_->schedule_begin_);

  auto trp0 = get_trip(*sched_, "0000004", 2, t.unix(10), "0000005", t.unix(11),
                       false, "foo");
  auto trp1 = get_trip(*sched_, "0000004", 2, t.unix(10), "0000005", t.unix(11),
                       false, "bar");

  ASSERT_NE(nullptr, trp0);
  ASSERT_NE(nullptr, trp1);
  ASSERT_NE(trp0, trp1);
}

TEST_F(loader_trip, rename) {
  time_helper t(sched_->schedule_begin_);
  auto trp0 = get_trip(*sched_, "0000001", 3, t.unix(20), "0000003", t.unix(22),
                       false, "");
  auto trp1 = get_trip(*sched_, "0000002", 4, t.unix(21), "0000003", t.unix(22),
                       false, "");

  ASSERT_NE(nullptr, trp0);
  ASSERT_NE(nullptr, trp1);
  ASSERT_EQ(trp0, trp1);
}

}  // loader
}  // motis
