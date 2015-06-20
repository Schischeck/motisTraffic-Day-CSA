#pragma once

#include <string>

#include "motis/core/schedule/schedule.h"

namespace motis {

int serialize(text_schedule const& sched, std::string const& prefix);

}  // namespace motis
