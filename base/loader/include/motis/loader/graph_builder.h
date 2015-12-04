#pragma once

#include <ctime>

#include "motis/core/schedule/schedule.h"

#include "motis/schedule-format/Schedule_generated.h"

namespace motis {
namespace loader {

schedule_ptr build_graph(Schedule const* serialized, time_t from, time_t to,
                         bool check_unique, bool apply_rules);

}  // namespace loader
}  // namespace motis
