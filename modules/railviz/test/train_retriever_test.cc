#include "gtest/gtest.h"
#include <vector>
#include "motis/core/common/date_util.h"
#include "motis/railviz/train_retriever.h"
#include "motis/loader/loader.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/synced_schedule.h"
#include "motis/railviz/geo.h"
using namespace motis::railviz;
using namespace motis;

TEST(railviz_all_train, two_trains_driving) {
  auto schedule = loader::load_schedule(
      {"modules/railviz/test/test_timetables/schedule_loop/", false, false,
       false, "20151121", 2});
  train_retriever ttr(*schedule);
  constexpr motis::time offset = MINUTES_A_DAY + 600;
  geo::box area = {{49.8728, 8.24411}, {50.969, 9.79821}};

  auto trains = ttr.trains(offset, offset + 10, 1000, area);
  ASSERT_NE(0, trains.size());
  ASSERT_EQ(2, trains.size());
  EXPECT_EQ(offset, trains[0].first->d_time);
  EXPECT_EQ(offset + 10, trains[1].first->d_time);
}
