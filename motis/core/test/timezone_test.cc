#include "gtest/gtest.h"

#include "motis/core/schedule/timezone.h"

namespace motis {

// schedule begin time := 1440
// schedule interval := {}
// (unmodified) season interval := ()
// (final) season interval := []
// MAD := MINUTES_A_DAY == 1440
// invalid time := INV

TEST(core_timezone, gmt_plus_one) {
  auto const schedule_begin_idx = 0;
  auto const schedule_end_idx = 6;

  auto const general_offset = 60;
  auto const season_offset = 120;

  auto const season_begin_idx = 2;
  auto const season_end_idx = 4;

  auto const min_begin = 120;
  auto const min_end = 180;

  timezone tz = {general_offset,   season_offset,    schedule_begin_idx,
                 schedule_end_idx, season_begin_idx, season_end_idx,
                 min_begin,        min_end};

  ASSERT_EQ(general_offset, tz.general_offset_);
  ASSERT_EQ(season_offset, tz.season_.offset);

  // { MAD 2*MAD [ 3*MAD 4*MAD 5*MAD ] 6*MAD 7*MAD }
  ASSERT_EQ(3 * MINUTES_A_DAY + min_begin - general_offset, tz.season_.begin);
  ASSERT_EQ(5 * MINUTES_A_DAY + min_end - season_offset, tz.season_.end);

  ASSERT_EQ(MINUTES_A_DAY, tz.to_motis_time(0, 60));
  ASSERT_EQ(5 * MINUTES_A_DAY + 180, tz.to_motis_time(4, 240));
}

TEST(core_timezone, gmt_minus_one) {
  auto const schedule_begin_idx = 0;
  auto const schedule_end_idx = 6;

  auto const general_offset = -60;
  auto const season_offset = -120;

  auto const season_begin_idx = 2;
  auto const season_end_idx = 4;

  auto const min_begin = 0;
  auto const min_end = 60;

  timezone tz = {general_offset,   season_offset,    schedule_begin_idx,
                 schedule_end_idx, season_begin_idx, season_end_idx,
                 min_begin,        min_end};

  ASSERT_EQ(general_offset, tz.general_offset_);
  ASSERT_EQ(season_offset, tz.season_.offset);

  // { MAD 2*MAD [ 3*MAD 4*MAD 5*MAD ] 6*MAD 7*MAD }
  ASSERT_EQ(3 * MINUTES_A_DAY + min_begin - general_offset, tz.season_.begin);
  ASSERT_EQ(5 * MINUTES_A_DAY + min_end - season_offset, tz.season_.end);

  ASSERT_EQ(MINUTES_A_DAY + 120, tz.to_motis_time(0, 60));
  ASSERT_EQ(5 * MINUTES_A_DAY + 180, tz.to_motis_time(4, 120));
}

TEST(core_timezone, season_outside_schedule_period) {
  auto const schedule_begin_idx = 1;
  auto const schedule_end_idx = 5;

  auto const general_offset = 60;
  auto const season_offset = 120;

  auto const season_begin_idx = 0;
  auto const season_end_idx = 6;

  auto const min_begin = 120;
  auto const min_end = 180;

  timezone tz = {general_offset,   season_offset,    schedule_begin_idx,
                 schedule_end_idx, season_begin_idx, season_end_idx,
                 min_begin,        min_end};

  // ( INV [{ 1*MAD  2*MAD 3*MAD 4*MAD 5*MAD }] INV )
  ASSERT_EQ(MINUTES_A_DAY + min_begin - general_offset, tz.season_.begin);
  ASSERT_EQ(5 * MINUTES_A_DAY + min_end - season_offset, tz.season_.end);
}
}
