#pragma once

#include "motis/core/common/journey.h"

namespace motis {
class label;
struct schedule;

journey to_journey(label const* label, schedule const& sched);

}  // namespace motis
