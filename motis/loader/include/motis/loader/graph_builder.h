#pragma once

#include "motis/core/schedule/schedule.h"

#include "motis/schedule-format/Schedule_generated.h"

namespace motis {
namespace loader {

schedule_ptr build_graph(Schedule const* serialized);

}  // namespace loader
}  // namespace motis
