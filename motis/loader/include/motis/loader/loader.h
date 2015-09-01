#pragma once

#include <ctime>

#include "motis/core/schedule/schedule.h"

namespace motis {
namespace loader {

schedule_ptr load_schedule(std::string const& path, time_t from, time_t to);

}  // namespace loader
}  // namespace motis
