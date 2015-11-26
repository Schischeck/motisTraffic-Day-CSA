#pragma once

#include "motis/core/schedule/timezone.h"

namespace motis {
namespace loader {

timezone create_timezone(int general_offset, int season_offset,
                         int day_idx_schedule_first_day,
                         int day_idx_schedule_last_day,
                         int day_idx_season_first_day,
                         int day_idx_season_last_day,
                         int minutes_after_midnight_season_begin,
                         int minutes_after_midnight_season_end);

}  // namspace loader
}  // namespace motis
