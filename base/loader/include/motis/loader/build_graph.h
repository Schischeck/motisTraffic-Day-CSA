#pragma once

#include <ctime>

#include "motis/core/schedule/schedule.h"

namespace motis {
namespace loader {

struct Schedule;

schedule_ptr build_graph(Schedule const* serialized, time_t from, time_t to,
                         bool unique_check, bool apply_rules,
                         bool adjust_footpaths);

}  // namespace loader
}  // namespace motis
