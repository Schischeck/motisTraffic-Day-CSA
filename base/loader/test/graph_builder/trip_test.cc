#include "gtest/gtest.h"

#include "./graph_builder_test.h"

#include "motis/core/common/date_util.h"
#include "motis/core/access/trip_access.h"

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

  ASSERT_ANY_THROW(get_trip(*sched_, "1234567", t.unix(1), 31337, "", "7654321",
                            t.unix(2), true));
}

TEST_F(loader_trip, simple) {
  time_helper t(sched_->schedule_begin_);
  auto trp = get_trip(*sched_, "0000001", t.unix(10), 1, "", "0000003",
                      t.unix(12), false);
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

  ASSERT_NE(nullptr, trp->first_route_edge);
  ASSERT_EQ(edge::ROUTE_EDGE, trp->first_route_edge->type());

  auto const& conns = trp->first_route_edge->_m._route_edge._conns;
  ASSERT_TRUE(trp->lcon_idx < conns.size());

  EXPECT_EQ(t.motis(10), conns[trp->lcon_idx].d_time);
  EXPECT_EQ(t.motis(11), conns[trp->lcon_idx].a_time);
}

TEST_F(loader_trip, collision) {
  time_helper t(sched_->schedule_begin_);

  auto trp0 = get_trip(*sched_, "0000004", t.unix(10), 2, "foo", "0000005",
                       t.unix(11), false);
  auto trp1 = get_trip(*sched_, "0000004", t.unix(10), 2, "bar", "0000005",
                       t.unix(11), false);

  ASSERT_NE(nullptr, trp0);
  ASSERT_NE(nullptr, trp1);
  ASSERT_NE(trp0, trp1);
}

// TEST_F(loader_trip, rename) {
//   time_helper t(sched_->schedule_begin_);
//   auto trp0 = get_trip(*sched_, "0000001", t.unix(20), 3, "", "0000003",
//                        t.unix(22), false);
//   auto trp1 = get_trip(*sched_, "0000002", t.unix(21), 4, "", "0000003",
//                        t.unix(22), false);

//   ASSERT_NE(nullptr, trp0);
//   ASSERT_NE(nullptr, trp1);
//   ASSERT_EQ(trp0, trp1);
// }

}  // loader
}  // motis
