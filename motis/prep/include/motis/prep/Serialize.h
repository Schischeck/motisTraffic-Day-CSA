#pragma once

#include <string>

#include "motis/core/schedule/schedule.h"

namespace td {

int serialize(text_schedule const& sched, std::string const& prefix);

}  // namespace td
