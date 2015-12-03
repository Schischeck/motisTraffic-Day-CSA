#include "gtest/gtest.h"

#include "motis/core/schedule/time.h"
#include "motis/core/schedule/timezone.h"

#include "motis/loader/timezone_util.h"

namespace motis {
namespace loader {

// schedule begin time := 1440
// schedule interval := {}
// (unmodified) season interval := ()
// (final) season interval := []
// MAD := MINUTES_A_DAY == 1440
// invalid time := INV

TEST(core_timezone, gmt_plus_one) {
  auto const general_offset = 60;
  auto const season_offset = 120;

  auto const day_idx_schedule_first_day = 0;
  auto const day_idx_schedule_last_day = 6;

  auto const day_idx_season_first_day = 2;
  auto const day_idx_season_last_day = 4;

  auto const minutes_after_midnight_season_begin = 120;
  auto const minutes_after_midnight_season_end = 180;

  timezone const tz = create_timezone(
      general_offset, season_offset, day_idx_schedule_first_day,
      day_idx_schedule_last_day, day_idx_season_first_day,
      day_idx_season_last_day, minutes_after_midnight_season_begin,
      minutes_after_midnight_season_end);

  ASSERT_EQ(general_offset, tz.general_offset_);
  ASSERT_EQ(season_offset, tz.season_.offset);

  // { MAD 2*MAD [ 3*MAD 4*MAD 5*MAD ] 6*MAD 7*MAD }
  ASSERT_EQ(
      3 * MINUTES_A_DAY + minutes_after_midnight_season_begin - general_offset,
      tz.season_.begin);
  ASSERT_EQ(
      5 * MINUTES_A_DAY + minutes_after_midnight_season_end - season_offset,
      tz.season_.end);

  ASSERT_EQ(MINUTES_A_DAY, tz.to_motis_time(0, 60));
  ASSERT_EQ(5 * MINUTES_A_DAY + 180, tz.to_motis_time(4, 240));
}

TEST(core_timezone, gmt_minus_one) {
  auto const general_offset = -60;
  auto const season_offset = -120;

  auto const day_idx_schedule_first_day = 0;
  auto const day_idx_schedule_last_day = 6;

  auto const day_idx_season_first_day = 2;
  auto const day_idx_season_last_day = 4;

  auto const minutes_after_midnight_season_begin = 0;
  auto const minutes_after_midnight_season_end = 60;

  timezone const tz = create_timezone(
      general_offset, season_offset, day_idx_schedule_first_day,
      day_idx_schedule_last_day, day_idx_season_first_day,
      day_idx_season_last_day, minutes_after_midnight_season_begin,
      minutes_after_midnight_season_end);

  ASSERT_EQ(general_offset, tz.general_offset_);
  ASSERT_EQ(season_offset, tz.season_.offset);

  // { MAD 2*MAD [ 3*MAD 4*MAD 5*MAD ] 6*MAD 7*MAD }
  ASSERT_EQ(
      3 * MINUTES_A_DAY + minutes_after_midnight_season_begin - general_offset,
      tz.season_.begin);
  ASSERT_EQ(
      5 * MINUTES_A_DAY + minutes_after_midnight_season_end - season_offset,
      tz.season_.end);

  ASSERT_EQ(MINUTES_A_DAY + 120, tz.to_motis_time(0, 60));
  ASSERT_EQ(5 * MINUTES_A_DAY + 180, tz.to_motis_time(4, 120));
}

TEST(core_timezone, season_begin_end_overlaps_schedule_period) {
  auto const general_offset = 60;
  auto const season_offset = 120;

  auto const day_idx_schedule_first_day = 1;
  auto const day_idx_schedule_last_day = 5;

  auto const day_idx_season_first_day = 0;
  auto const day_idx_season_last_day = 6;

  auto const minutes_after_midnight_season_begin = 120;
  auto const minutes_after_midnight_season_end = 180;

  timezone const tz = create_timezone(
      general_offset, season_offset, day_idx_schedule_first_day,
      day_idx_schedule_last_day, day_idx_season_first_day,
      day_idx_season_last_day, minutes_after_midnight_season_begin,
      minutes_after_midnight_season_end);

  // from: [ INV {  1*MAD  2*MAD 3*MAD 4*MAD 5*MAD }  INV ]
  // to:     INV [{ 1*MAD  2*MAD 3*MAD 4*MAD 5*MAD }] INV
  ASSERT_EQ(0, tz.season_.begin);
  ASSERT_EQ(INVALID_TIME - season_offset, tz.season_.end);
}

TEST(core_timezone, season_end_before_schedule_period) {
  auto const general_offset = 60;
  auto const season_offset = 120;

  auto const day_idx_schedule_first_day = 3;
  auto const day_idx_schedule_last_day = 6;

  auto const day_idx_season_first_day = 2;
  auto const day_idx_season_last_day = 1;

  auto const minutes_after_midnight_season_begin = 120;
  auto const minutes_after_midnight_season_end = 180;

  timezone const tz = create_timezone(
      general_offset, season_offset, day_idx_schedule_first_day,
      day_idx_schedule_last_day, day_idx_season_first_day,
      day_idx_season_last_day, minutes_after_midnight_season_begin,
      minutes_after_midnight_season_end);

  //  [ INV INV INV ] { 1*MAD  2*MAD 3*MAD 4*MAD }
  ASSERT_EQ(INVALID_TIME, tz.season_.begin);
  ASSERT_EQ(INVALID_TIME, tz.season_.end);
  ASSERT_EQ(INVALID_TIME, tz.season_.offset);
}

TEST(core_timezone, season_begin_after_schedule_period) {
  auto const general_offset = 60;
  auto const season_offset = 120;

  auto const day_idx_schedule_first_day = 2;
  auto const day_idx_schedule_last_day = 6;

  auto const day_idx_season_first_day = 0;
  auto const day_idx_season_last_day = 1;

  auto const minutes_after_midnight_season_begin = 120;
  auto const minutes_after_midnight_season_end = 180;

  timezone const tz = create_timezone(
      general_offset, season_offset, day_idx_schedule_first_day,
      day_idx_schedule_last_day, day_idx_season_first_day,
      day_idx_season_last_day, minutes_after_midnight_season_begin,
      minutes_after_midnight_season_end);

  //  { 1*MAD  2*MAD 3*MAD 4*MAD } [ INV INV INV ]
  ASSERT_EQ(INVALID_TIME, tz.season_.begin);
  ASSERT_EQ(INVALID_TIME, tz.season_.end);
  ASSERT_EQ(INVALID_TIME, tz.season_.offset);
}

TEST(core_timezone, move_season_begin_to_schedule_period_begin) {
  auto const general_offset = 60;
  auto const season_offset = 120;

  auto const day_idx_schedule_first_day = 1;
  auto const day_idx_schedule_last_day = 4;

  auto const day_idx_season_first_day = 0;
  auto const day_idx_season_last_day = 2;

  auto const minutes_after_midnight_season_begin = 120;
  auto const minutes_after_midnight_season_end = 180;

  timezone const tz = create_timezone(
      general_offset, season_offset, day_idx_schedule_first_day,
      day_idx_schedule_last_day, day_idx_season_first_day,
      day_idx_season_last_day, minutes_after_midnight_season_begin,
      minutes_after_midnight_season_end);

  // from: [ INV {  1*MAD  2*MAD ] 3*MAD 4*MAD } INV INV
  // to:     INV [{ 1*MAD  2*MAD ] 3*MAD 4*MAD } INV INV
  ASSERT_EQ(0, tz.season_.begin);
  ASSERT_EQ(
      2 * MINUTES_A_DAY + minutes_after_midnight_season_end - season_offset,
      tz.season_.end);
}

TEST(core_timezone, move_season_end_to_schedule_period_end) {
  auto const general_offset = 60;
  auto const season_offset = 120;

  auto const day_idx_schedule_first_day = 1;
  auto const day_idx_schedule_last_day = 4;

  auto const day_idx_season_first_day = 3;
  auto const day_idx_season_last_day = 5;

  auto const minutes_after_midnight_season_begin = 120;
  auto const minutes_after_midnight_season_end = 180;

  timezone const tz = create_timezone(
      general_offset, season_offset, day_idx_schedule_first_day,
      day_idx_schedule_last_day, day_idx_season_first_day,
      day_idx_season_last_day, minutes_after_midnight_season_begin,
      minutes_after_midnight_season_end);

  // from:   INV { 1*MAD  2*MAD [ 3*MAD 4*MAD }  INV ] INV
  // to:     INV { 1*MAD  2*MAD [ 3*MAD 4*MAD }] INV   INV
  ASSERT_EQ(
      3 * MINUTES_A_DAY + minutes_after_midnight_season_begin - general_offset,
      tz.season_.begin);
  ASSERT_EQ(INVALID_TIME - season_offset, tz.season_.end);
}

}  // namespace loader
}  // namespace motis
