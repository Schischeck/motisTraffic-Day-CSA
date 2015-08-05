#pragma once

#include "motis/core/schedule/schedule.h"

namespace motis {
namespace loader {

inline schedule_ptr load_schedule(std::string const&) { return {}; }

}  // namespace loader
}  // namespace motis