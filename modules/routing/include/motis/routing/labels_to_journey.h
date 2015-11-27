#pragma once

#include "motis/core/journey/journey.h"

namespace motis {
class label;
struct schedule;

journey labels_to_journey(label const* label, schedule const& sched);

}  // namespace motis
