#pragma once

#include "motis/core/schedule/schedule.h"

namespace motis {

schedule_ptr load_schedule(std::string const& prefix);
schedule_ptr load_text_schedule(std::string const& prefix);
schedule_ptr load_binary_schedule(std::string const& prefix);

}  // namespace motis
